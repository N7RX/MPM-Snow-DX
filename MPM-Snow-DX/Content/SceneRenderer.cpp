#include "pch.h"
#include "SceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace MPM_Snow_DX;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(0),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		//float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		//double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		//float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		//Rotate(radians);
		Rotate(0); // Setup MVP, static

		// First time only
		SetupScene();

		// Update MPM snow simulator
		m_snowSimulator->update();
		// Acquire updated vertices from point cloud
		AcquireVertices();		
	}
}

// Rotate the 3D cube model a set amount of radians.
void SceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void SceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void SceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void SceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	// Setup dynamic buffer
	D3D11_MAPPED_SUBRESOURCE resource;
	context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, m_vertices, sizeof(VertexPositionColor) * m_snowSimulator->point_cloud->size);
	context->Unmap(m_vertexBuffer, 0);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		&m_vertexBuffer,
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our geometry shader
	context->GSSetShader(
		m_geometryShader.Get(),
		nullptr,
		0
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}

void SceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"VertexShader.cso");
	auto loadGSTask = DX::ReadDataAsync(L"GeometryShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"PixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});

	// After the geometry shader file is loaded, create the shader.
	auto createGSTask = loadGSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geometryShader
			)
		);

		// Any extra things to do?
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
	});

	// Once both shaders are loaded, create the mesh.
	auto createSnowTask = (createPSTask && createVSTask && createGSTask).then([this]() {

		SetupScene();

		// Load snow particles vertices. Each vertex has a position and a color.
		AcquireVertices();

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = m_vertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionColor) * m_snowSimulator->point_cloud->size, D3D11_BIND_VERTEX_BUFFER);

		// Set to dynamic
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		// Load particle indices.
		GenerateIndices();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = m_vertexIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(int) * m_indexCount, D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createSnowTask.then([this]() {
		m_loadingComplete = true;
	});
}

// Initialize scene content, scatter particles into the entities.
void SceneRenderer::SetupScene()
{
	if (m_snowSimulator == nullptr)
	{
		Scene* scene = Scene::GenerateScene(7); // Parameter: scene type
		m_snowSimulator = new Simulator(scene);
	}
}

// Update the latest position of the particles from the point cloud.
void SceneRenderer::AcquireVertices()
{
	if (m_vertices != nullptr)
	{
		delete[] m_vertices;
	}
	m_vertices = new VertexPositionColor[m_snowSimulator->point_cloud->size];

	for (int i = 0; i < m_snowSimulator->point_cloud->size; i++)
	{
		Particle& p = m_snowSimulator->point_cloud->particles[i];
		// Use the particle's density to vary color
		float contrast = 0.5f;
		float density = p.density / DENSITY * contrast;
		density += 1 - contrast;

		m_vertices[i] = {
			XMFLOAT3(( // Add an offset to translate the particles into the center of view field
				(float)p.position(0) - 1) * 2,	// x
				(float)p.position(1) * 2 - 1,	// y
				(float)p.position(2) * 2 - 1),  // z
			XMFLOAT4(density * 0.9f, density * 0.95f, density, p.volume) }; // color & volume
	}
}

// Create render index for particles.
void SceneRenderer::GenerateIndices()
{
	m_indexCount = m_snowSimulator->point_cloud->size * 2; // Hmmmmm... actually I don't know why it has to be multiplied by 2 to have the correct number of indices
	if (m_vertexIndices != nullptr)
	{
		delete[] m_vertexIndices;
	}
	m_vertexIndices = new int[m_indexCount];

	for (int i = 0; i < m_indexCount; i++)
	{
		m_vertexIndices[i] = i; // Just draw by the index, filling job would be done in Geo Shader
	}
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer->Release();
	m_indexBuffer.Reset();
}

int SceneRenderer::GetParticleCount()
{
	if (m_snowSimulator != nullptr)
	{
		return m_snowSimulator->point_cloud->size;
	}
	return 0;
}