#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 textureCoord;

// location is the frame buffer index.
// out declares the variable as the one for output of the fragment shader.
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(textureCoord, 0.0, 1.0);
}