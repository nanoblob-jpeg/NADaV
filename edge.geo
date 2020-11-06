#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT{
	vec4 color;
	vec4 pos1;
	vec4 pos2;
} gs_in[];
out vec4 vertexColor;
void main(){
	vertexColor = gs_in[0].color;
	gl_Position = gs_in[0].pos1;
	EmitVertex();

	gl_Position = gs_in[0].pos2;
	EmitVertex();
	EndPrimitive();
}