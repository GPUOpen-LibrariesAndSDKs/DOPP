#version 420 core

layout(location = 0) in vec4 inVertex;
layout(location = 4) in vec2 inTexCoord;

out vec2 TexCoord;

void main( void )
{
    gl_Position = inVertex;
    TexCoord    = inTexCoord;
}
