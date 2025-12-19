#version 450

// Output variable for the vertex color
layout(location = 0) out vec3 vertexColor;

// Vertices in clipped coordinates
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

// Colors of the vertices, in RGB
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0), // red
    vec3(0.0, 1.0, 0.0), // green
    vec3(0.0, 0.0, 1.0) // blue
);

// Main function for the vexter shader.
// It is called for each vertex.
void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    vertexColor = colors[gl_VertexIndex];
}