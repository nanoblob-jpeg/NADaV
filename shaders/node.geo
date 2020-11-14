#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT{
	vec4 color;
} gs_in[];
out vec4 vertexColor;
void build_box(vec4 position)
{
	vertexColor = gs_in[0].color;
	gl_Position = position + vec4(-0.05, -0.05, 0.0, 0.0);
    EmitVertex();
    gl_Position = position + vec4( 0.05, -0.05, 0.0, 0.0);
    EmitVertex();
    gl_Position = position + vec4(-0.05,  0.05, 0.0, 0.0);
    EmitVertex();
    gl_Position = position + vec4( 0.05,  0.05, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
}

void main(){
	build_box(gl_in[0].gl_Position);
}