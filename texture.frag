// texture.frag
#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D imageTexture; // Signature PNG
uniform vec4 color;             // Boja/Alpha (1.0, 1.0, 1.0, 0.75)

void main()
{
    // Uzorkujemo boju iz PNG teksture
    vec4 texColor = texture(imageTexture, TexCoord);
    
    // Konačni Alpha kanal je proizvod alfa iz teksture i alfa iz C++
    // RGB je boja iz PNG-a pomnožena sa belom (da zadrži boju)
    FragColor = vec4(texColor.rgb * color.rgb, texColor.a * color.a);
}