#include "pch.h"
#include "StatusTextRenderer.h"

#include "Common/DirectXHelper.h"

using namespace MPM_Snow_DX;
using namespace Microsoft::WRL;

// Initializes D2D resources used for text rendering.
StatusTextRenderer::StatusTextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_text(L""),
	m_deviceResources(deviceResources)
{
	ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

	// Create device independent resources
	ComPtr<IDWriteTextFormat> textFormat;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			28.0f,
			L"en-US",
			&textFormat
		)
	);

	DX::ThrowIfFailed(
		textFormat.As(&m_textFormat)
	);

	DX::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
	);

	CreateDeviceDependentResources();
}

// Updates the text to be displayed.
void StatusTextRenderer::Update(DX::StepTimer const& timer)
{
	// Update display text.
	uint32 fps = timer.GetFramesPerSecond();

	m_text = 
		(L"Particles: " + ((m_particleCount > 0) ? std::to_wstring(m_particleCount) + L"  " : L" -  "))
		+ (L"FPS: " + ((fps > 0) ? std::to_wstring(fps) : L"-"));

	ComPtr<IDWriteTextLayout> textLayout;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			m_text.c_str(),
			(uint32)m_text.length(),
			m_textFormat.Get(),
			400.0f, // Max width of the input text.
			50.0f, // Max height of the input text.
			&textLayout
		)
	);

	DX::ThrowIfFailed(
		textLayout.As(&m_textLayout)
	);

	DX::ThrowIfFailed(
		m_textLayout->GetMetrics(&m_textMetrics)
	);
}

// Renders a frame to the screen.
void StatusTextRenderer::Render()
{
	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	// Position on the bottom right corner
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		(logicalSize.Width - m_textMetrics.layoutWidth) / 2,
		logicalSize.Height - m_textMetrics.height
	);

	context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());

	DX::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)
	);

	context->DrawTextLayout(
		D2D1::Point2F(0.f, 0.f),
		m_textLayout.Get(),
		m_whiteBrush.Get()
	);

	// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());
}

void StatusTextRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
	);
}
void StatusTextRenderer::ReleaseDeviceDependentResources()
{
	m_whiteBrush.Reset();
}

void StatusTextRenderer::SetDynamicStatus(int particleCount)
{
	m_particleCount = particleCount;
}