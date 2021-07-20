#version 420 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D ourTexture;
uniform vec4 texColor;

void main()
{
	fragColor = texture(ourTexture, texCoord) * texColor;
}