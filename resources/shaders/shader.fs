#version 450 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D atlasTex;
uniform sampler2D modelTex;

uniform vec4 texColor;
uniform vec3 uiColor;
uniform int mode;

uniform float timer;

vec4 divideAlpha(vec4 original)
{
	float alpha = original.a;
	vec4 newColor = original / original.a;
	newColor.a = alpha;
	return newColor;
}

vec4 applyEffect (vec4 original)
{
	float converted = mod((timer * 4.0f + texCoord.y * 5.0f), 6.0f);
	vec4 newColor = mix(original, vec4(clamp(-1.0f + abs(3.0f - converted), 0.0f, 1.0f), clamp(2.0f - abs(2.0f - converted), 0.0f, 1.0f), clamp(2.0f - abs(4.0f - converted), 0.0f, 1.0f), 1.0f), original.a * 0.7f);
	return newColor;
}

void main()
{		
	switch (mode)
	{
		case(0):
			fragColor = texture(atlasTex, texCoord) * texColor;
			break;
		case(1):
			fragColor = vec4(uiColor, 1.0f);
			break;
		case(2):
			fragColor = texture(modelTex, texCoord) * texColor;
			break;
		case(3):
			fragColor = applyEffect(texture(modelTex, texCoord) * texColor);
			break;
		case(4):
			fragColor = divideAlpha(texture(modelTex, texCoord) * texColor);
			break;
		case(5):
			fragColor = applyEffect(divideAlpha(texture(modelTex, texCoord) * texColor));
			break;
		default:
			break;
	}
}