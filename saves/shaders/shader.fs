#version 420 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D ourTexture;
uniform vec4 texColor;

uniform vec3 uiColor;
uniform bool drawPoints;

void main()
{
	if (!drawPoints)
		fragColor = texture(ourTexture, texCoord) * texColor;
	else
		fragColor = vec4(uiColor, 1.0f);
}