#version 460 core

in vec2 TexCoord;

uniform sampler2D Texture;
uniform int state;

void main()
{
	vec2 texSize = textureSize(Texture, 0);
	if (gl_FragCoord.x/texSize.x < 1.0 && gl_FragCoord.x/texSize.x > 0.96 && gl_FragCoord.y/texSize.y < 1.0 && gl_FragCoord.y/texSize.y > 0.96){
		if (state == 0) {
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		}
		else if (state == 1) {
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		} 
		else if (state == 2) {
			gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		}
	}
	else{
		gl_FragColor = texture(Texture, TexCoord);
	}
}