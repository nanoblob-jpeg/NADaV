#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aColor;

out VS_OUT{
	vec4 color;
} vs_out;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vs_out.color = vec4(aColor.x, 0.0,aColor.y, 1.0);
	gl_Position = proj * view * vec4(aPos, 0.0, 1.0);
}