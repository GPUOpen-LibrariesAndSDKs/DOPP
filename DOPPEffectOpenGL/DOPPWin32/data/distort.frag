#version 420 core

uniform float     fTime;
uniform sampler2D baseMap;

in vec2 TexCoord;

void main( void )
{
    float d;
    ivec2  texSize = textureSize(baseMap, 0);     
       
    d = distance(texSize/2.0, vec2(gl_FragCoord.x, gl_FragCoord.y));
    
    float s = sin(d/(texSize.x/8.0)) * sin(fTime)/10.0;
     
    gl_FragColor = texture2D( baseMap, vec2(TexCoord.s + s, TexCoord.t) );
   
    
}