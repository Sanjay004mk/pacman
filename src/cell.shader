$TYPE VERTEX
#version 450

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords;

layout(location = 0) smooth out vec2 fTexCoords;

uniform mat4 proj;

void main()
{
	fTexCoords = aTexCoords;
	gl_Position = proj * vec4(aPosition, 0.0f, 1.0f);
}

$TYPE FRAGMENT
#version 450

out vec4 color;

layout(location = 0) smooth in vec2 fTexCoords;

void main()
{
	color = vec4(fTexCoords, 0.0f, 1.0f);
}