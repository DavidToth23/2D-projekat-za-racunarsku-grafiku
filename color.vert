#version 330 core

layout (location = 0) in vec2 aPos;

uniform float positionX;
uniform float positionY;
uniform float width;
uniform float height;

void main()
{
    // aPos je u opsegu [-1, 1].
    // Skaliranje (width, height) i translacija (positionX, positionY)
    vec2 pos = aPos * vec2(width, height) + vec2(positionX, positionY);
    gl_Position = vec4(pos, 0.0, 1.0);
}