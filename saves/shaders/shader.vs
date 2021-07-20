#version 420 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 transform;
uniform mat4 projection;

void main()
{
	gl_Position = projection * transform * vec4(aPos, 0.0f, 1.0f);
	texCoord = aTexCoord;
}