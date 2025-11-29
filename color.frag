#version 330 core
out vec4 FragColor;

uniform vec4 color; // Boja koju šaljemo iz C++

void main()
{
    FragColor = color;
}