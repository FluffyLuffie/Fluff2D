#version 420 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D tex;

uniform vec4 texColor;
uniform vec3 uiColor;
uniform int mode;

uniform float timer;

void main()
{	
	fragColor = texture(tex, texCoord) * texColor;
		
	switch (mode)
	{
		case(1):
			fragColor = vec4(uiColor, 1.0f);
			break;
		case(2):
			fragColor = fragColor / fragColor.a;
			fragColor.a = texture(tex, texCoord).a;
			break;
		case(3):
			float converted = mod((timer + texCoord.y * 5.0f), 6.0f);
			fragColor = mix(fragColor, vec4(clamp(-1.0f + abs(3.0f - converted), 0.0f, 1.0f), clamp(2.0f - abs(2.0f - converted), 0.0f, 1.0f), clamp(2.0f - abs(4.0f - converted), 0.0f, 1.0f), 1.0f), fragColor.a * 0.7f);
			break;
		case (4):
			float converted2 = mod((timer + texCoord.y * 5.0f), 6.0f);
			fragColor = mix(fragColor, vec4(clamp(-1.0f + abs(3.0f - converted2), 0.0f, 1.0f), clamp(2.0f - abs(2.0f - converted2), 0.0f, 1.0f), clamp(2.0f - abs(4.0f - converted2), 0.0f, 1.0f), 1.0f), fragColor.a * 0.7f);
			
			fragColor = fragColor / fragColor.a;
			fragColor.a = texture(tex, texCoord).a;
			break;
		default:
			break;
	}
}