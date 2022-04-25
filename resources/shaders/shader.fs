#version 330 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int meshID;

in vec2 texCoord;

uniform sampler2D atlasTex;
uniform sampler2D modelTex;

uniform vec4 texColor;
uniform vec3 uiColor;
uniform int mode;
uniform int ID;

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
			fragColor.r *= texColor.a;
			fragColor.g *= texColor.a;
			fragColor.b *= texColor.a;
			break;
		case(1):
			fragColor = vec4(uiColor, 1.0f);
			break;
		case(2):
			fragColor = texture(atlasTex, texCoord) * texColor;
			fragColor.r *= texColor.a / fragColor.a;
			fragColor.g *= texColor.a / fragColor.a;
			fragColor.b *= texColor.a / fragColor.a;
			break;
		case(3):
			fragColor = vec4(0.0f, 0.0f, 0.0f, texture(atlasTex, texCoord).a);
			if (fragColor.a == 0.0f)
				discard;
			meshID = ID;
			break;
		case(4):
			fragColor = texture(atlasTex, texCoord) * texColor;
			fragColor.r *= texColor.a / fragColor.a;
			fragColor.g *= texColor.a / fragColor.a;
			fragColor.b *= texColor.a / fragColor.a;
			
			fragColor.r = 1.0f - fragColor.r;
			fragColor.g = 1.0f - fragColor.g;
			fragColor.b = 1.0f - fragColor.b;
			break;
		case(5):
			vec4 v = texture(modelTex, texCoord);
			fragColor = v / v.a;
			fragColor.a = v.a;
			break;
		default:
			break;
	}
}