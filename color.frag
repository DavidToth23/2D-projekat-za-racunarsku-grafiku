#version 330 core
out vec4 FragColor;

uniform vec4 color;

uniform int useTexture; // 0=koristi boju, 1=koristi teksturu
uniform sampler2D textureSampler;

in vec2 TexCoord;

void main()
{
    if (useTexture == 1) {
        vec4 sampled = texture(textureSampler, TexCoord);

        // odbacujemo belu pozadinu na ekg slici
        // ako je prosek RGB blizu 1 to je bela
        float brightness = (sampled.r + sampled.g + sampled.b) / 3.0;

        // ako je piksel skoro beo odbacujemo ga
        if (brightness > 0.8) { 
            discard;
        }
        
        //iscrtavamo crvenu liniju
        FragColor = vec4(color.rgb, color.a * sampled.r);
        
    } else {
        FragColor = color;
    }
}