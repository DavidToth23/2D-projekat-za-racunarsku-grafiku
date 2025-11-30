#version 330 core
out vec4 FragColor;

uniform vec4 color; // Boja koju šaljemo iz C++

uniform int useTexture; // 0=koristi boju, 1=koristi teksturu
uniform sampler2D textureSampler;

in vec2 TexCoord; // UV koordinata sa Vertex Shadera

void main()
{
    if (useTexture == 1) {
        vec4 sampled = texture(textureSampler, TexCoord);

        // 1. Provera EKG Linije vs Pozadina (Bela)
        // Ako je boja blizu bele (pozadina EKG-a), odbacujemo je.
        // EKG Linija je crvena (R=1.0, G=0.0, B=0.0). Pozadina je svetlo siva/bela.
        
        // Uzmimo prosek R, G, B. Ako je prosek visok (blizu 1.0), to je pozadina.
        float brightness = (sampled.r + sampled.g + sampled.b) / 3.0;

        // Ako je piksel skoro beo (threshold 0.8), odbaci fragment
        if (brightness > 0.8) { 
            discard; // Odbacuje fragmente bele/sive pozadine
        }
        
        // 2. Crtanje Linije
        // Ako je sampled.r (crvena komponenta) blizu 1.0, crtamo punom uniform bojom.
        // FragColor = vec4(color.rgb, color.a); // Koristi uniform boju (koju smo postavili na crvenu)

        // Bolje: koristimo crvenu uniform boju i alfa vrednost iz teksture
        FragColor = vec4(color.rgb, color.a * sampled.r);
        
    } else {
        // Standardno crtanje oblika
        FragColor = color;
    }
}