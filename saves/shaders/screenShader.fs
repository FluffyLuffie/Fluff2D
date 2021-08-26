#version 420 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;

uniform bool colorCorrection;
uniform bool effect;
uniform float timer;

void main()
{
	fragColor = texture(screenTexture, texCoord);
	
	if(effect)
	{
		float converted = mod((timer + texCoord.y * 5.0f), 6.0f);
		fragColor = mix(fragColor, vec4(clamp(-1.0f + abs(3.0f - converted), 0.0f, 1.0f), clamp(2.0f - abs(2.0f - converted), 0.0f, 1.0f), clamp(2.0f - abs(4.0f - converted), 0.0f, 1.0f), 1.0f), fragColor.a * 0.7f);
	}
	
	if(colorCorrection)
	{
		fragColor = fragColor / fragColor.a;
		fragColor.a = texture(screenTexture, texCoord).a;
	}
}