#version 330 core
layout (location = 0) in vec2 aPos;

uniform float positionX;
uniform float positionY;
uniform float width;
uniform float height;
uniform int drawShape; // 0=Quad, 1=Triangle Right, 2=Triangle Left
uniform float u_scroll; // Pomak X za animaciju
uniform float u_repeat; // Faktor ponavljanja teksture

out vec2 TexCoord;

void main()
{
    vec2 finalPos;
    vec2 posNDC;

    if (drawShape == 1) //desni trougao
    {
        if (gl_VertexID == 0) { 
             posNDC = vec2(-1.0, 1.0); 
        } else if (gl_VertexID == 1) { 
             posNDC = vec2(-1.0, -1.0); 
        } else { 
             posNDC = vec2(1.0, 0.0); 
        }
    }
    else if (drawShape == 2) //levi trougao
    {
        if (gl_VertexID == 0) { 
             posNDC = vec2(1.0, 1.0); 
        } else if (gl_VertexID == 1) { 
             posNDC = vec2(1.0, -1.0); 
        } else { 
             posNDC = vec2(-1.0, 0.0); 
        }
    }
    else //kvadrat
    {
        posNDC = aPos; 
    }

    // Skaliranje i translacija
    finalPos.x = posNDC.x * width + positionX;
    finalPos.y = posNDC.y * height + positionY;
    
    TexCoord.x = ((posNDC.x + 1.0) / 2.0) * u_repeat + u_scroll;
    TexCoord.y = (posNDC.y + 1.0) / 2.0;

    gl_Position = vec4(finalPos, 0.0, 1.0);
}