#version 460 core
layout (location = 0) in vec2 VertIn;
layout (location = 1) in vec2 TexCoordIn;

out vec2 TexCoord;
uniform mat4 transform;

void main()
{
	TexCoord = TexCoordIn;
	gl_Position = transform * vec4(VertIn, 0.0, 1.0);
}