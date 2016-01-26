#version 420 core

uniform sampler2D baseMap;

in vec2 TexCoord;

void main( void )
{
    vec4 texColor = texture2D( baseMap, TexCoord );
    
	gl_FragColor = vec4(1.0) - vec4(texColor.r, texColor.g, texColor.b, 0.0);
}