#version 330 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int meshId;

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
	meshId = 50;
	switch (mode)
	{
		case(0):
			fragColor = texture(atlasTex, texCoord) * texColor;
			fragColor.r *= texColor.a;
			fragColor.g *= texColor.a;
			fragColor.b *= texColor.a;
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
			fragColor = divideAlpha(applyEffect(texture(modelTex, texCoord) * texColor));
			break;
		case(6):
			vec4 v = texture(atlasTex, texCoord) * texColor;
			fragColor = v / v.a;
			fragColor.r *= texColor.a;
			fragColor.g *= texColor.a;
			fragColor.b *= texColor.a;
			fragColor.a = v.a;
			break;
		case(7):
			vec4 v2 = texture(atlasTex, texCoord) * texColor;
			if (v2.a <= 0.0f){
				discard;
			}
			fragColor = v2;
			break;
		case(8):
			fragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		default:
			break;
	}
}