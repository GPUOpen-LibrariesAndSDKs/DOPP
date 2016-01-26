
#version 420 core

layout(location = 0) in vec4 inVertex;
layout(location = 4) in vec2 inTexCoord;

layout(shared, binding = 0) uniform FrameData
{
   mat4		ModelViewMat;
   mat4		ProjectionMat;
};

out vec2 TexCoord;


void main()
{
   TexCoord = inTexCoord;

   gl_Position = ProjectionMat * ModelViewMat * inVertex;
}