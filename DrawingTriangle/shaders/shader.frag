#version 450

layout(location = 0) in vec3 fragColor;

// location is the frame buffer index.
// out declares the variable as the one for output of the fragment shader.
layout(location = 0) out vec4 outColor;

void main()
{
    // output a red color
    outColor = vec4(fragColor, 1.0);
}