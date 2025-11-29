#version 330 core
layout (location = 0) in vec2 aPos;

uniform float positionX;
uniform float positionY;
uniform float width;
uniform float height;
uniform int drawShape; // 0=Quad, 1=Triangle Right, 2=Triangle Left

void main()
{
    vec2 finalPos;
    vec2 posNDC;

    if (drawShape == 1) // TROUGAO DESNO
    {
        // P1: Gore levo (-1, 1), P2: Dole levo (-1, -1), P3: Desno centar (1, 0)
        if (gl_VertexID == 0) { 
             posNDC = vec2(-1.0, 1.0); 
        } else if (gl_VertexID == 1) { 
             posNDC = vec2(-1.0, -1.0); 
        } else { 
             posNDC = vec2(1.0, 0.0); 
        }
    }
    else if (drawShape == 2) // TROUGAO LEVO <--- NOVO
    {
        // P1: Gore desno (1, 1), P2: Dole desno (1, -1), P3: Levo centar (-1, 0)
        if (gl_VertexID == 0) { 
             posNDC = vec2(1.0, 1.0); 
        } else if (gl_VertexID == 1) { 
             posNDC = vec2(1.0, -1.0); 
        } else { 
             posNDC = vec2(-1.0, 0.0); 
        }
    }
    else // KVADRAT (drawShape == 0)
    {
        posNDC = aPos; 
    }

    // Skaliranje i translacija
    finalPos.x = posNDC.x * width + positionX;
    finalPos.y = posNDC.y * height + positionY;
    
    gl_Position = vec4(finalPos, 0.0, 1.0);
}