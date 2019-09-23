// Per-vertex data passed in and out the geometry shader.
// Set to the same with Pixel Shader to preserve vertex color.
struct GeometryShaderData
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

// Extend the vertex into a quad (in XY plane).
[maxvertexcount(6)]
void main(
	point GeometryShaderData input[1],
	inout TriangleStream< GeometryShaderData > output
)
{
	// Modify particle size according to density (color.b is equivalent to density)
	//float hl = 0.001f + 0.004f * input[0].color.b;
	float hl = input[0].color.a * 0.005f;

	// Clockwise - front face.
	// Upper-left triangle

	GeometryShaderData element;
	element.color = input[0].color;

	element.pos = input[0].pos + float4(hl, hl, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(-hl, -hl, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(-hl, hl, 0, 0);
	output.Append(element);

	output.RestartStrip();

	// Bottom-right triangle

	element.pos = input[0].pos + float4(hl, -hl, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(-hl, -hl, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(hl, hl, 0, 0);
	output.Append(element);

	//output.RestartStrip();
}