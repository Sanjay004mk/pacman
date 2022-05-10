$TYPE VERTEX
#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoords;

void main()
{
	gl_Position = vec4(aPosition, 1.0f);
}

$TYPE FRAGMENT
#version 450

out vec4 color;

void main()
{
	color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}