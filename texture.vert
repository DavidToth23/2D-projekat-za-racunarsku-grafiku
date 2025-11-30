// texture.vert
#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTexCoord; 
out vec2 TexCoord;

uniform mat4 transform; // Matrica za pozicioniranje i skaliranje

void main()
{
    // Primenjujemo transformaciju na verteks
    gl_Position = transform * vec4(inPos, 0.0, 1.0);
    TexCoord = inTexCoord;
}