#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 acolor;

out VS_OUT{
	vec4 color;
	vec4 pos1;
	vec4 pos2;
} vs_out;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vs_out.pos1 = proj * view * vec4(aPos.xy, 0.0, 1.0);
	vs_out.pos2 = proj * view * vec4(aPos.zw, 0.0, 1.0);
	vs_out.color = vec4(acolor.x, 0.0, acolor.y, 1.0);
}