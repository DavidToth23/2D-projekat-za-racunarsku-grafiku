#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// =========================================================
// EXTERN DEKLARACIJE GLOBALNIH PROMENLJIVIH IZ main.cpp
// =========================================================

// Shaderi i VAO/VBO za crtanje oblika
extern unsigned int colorShader;
extern unsigned int quadVAO;
extern unsigned int quadVBO;

// Geometrija okvira
extern const float FRAME_SIZE_X;
extern const float FRAME_SIZE_Y;

// =========================================================
// DEFINICIJE FUNKCIJA
// =========================================================

void initQuad() {
    float vertices[] = {
        // pozicija (x, y)
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f,  1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a) {
    glUseProgram(colorShader);

    glUniform1i(glGetUniformLocation(colorShader, "useTexture"), 0);

    int locX = glGetUniformLocation(colorShader, "positionX");
    int locY = glGetUniformLocation(colorShader, "positionY");
    int locW = glGetUniformLocation(colorShader, "width");
    int locH = glGetUniformLocation(colorShader, "height");
    int locC = glGetUniformLocation(colorShader, "color");

    glUniform1f(locX, x);
    glUniform1f(locY, y);
    glUniform1f(locW, width);
    glUniform1f(locH, height);
    glUniform4f(locC, r, g, b, a);

    int locShape = glGetUniformLocation(colorShader, "drawShape");
    glUniform1i(locShape, 0); // Kvadrat (drawShape=0)

    glUniform1f(glGetUniformLocation(colorShader, "u_scroll"), 0.0f);
    glUniform1f(glGetUniformLocation(colorShader, "u_repeat"), 1.0f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a) {
    glUseProgram(colorShader);

    glUniform1i(glGetUniformLocation(colorShader, "useTexture"), 0);

    int locX = glGetUniformLocation(colorShader, "positionX");
    int locY = glGetUniformLocation(colorShader, "positionY");
    int locW = glGetUniformLocation(colorShader, "width");
    int locH = glGetUniformLocation(colorShader, "height");
    int locC = glGetUniformLocation(colorShader, "color");

    glUniform1f(locX, x);
    glUniform1f(locY, y);
    glUniform1f(locW, width);
    glUniform1f(locH, height);
    glUniform4f(locC, r, g, b, a);

    int locShape = glGetUniformLocation(colorShader, "drawShape");
    glUniform1i(locShape, 1); // Trougao Desno (drawShape=1)

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUniform1i(locShape, 0);
    glBindVertexArray(0);
}

void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a) {
    glUseProgram(colorShader);

    glUniform1i(glGetUniformLocation(colorShader, "useTexture"), 0);

    int locX = glGetUniformLocation(colorShader, "positionX");
    int locY = glGetUniformLocation(colorShader, "positionY");
    int locW = glGetUniformLocation(colorShader, "width");
    int locH = glGetUniformLocation(colorShader, "height");
    int locC = glGetUniformLocation(colorShader, "color");

    glUniform1f(locX, x);
    glUniform1f(locY, y);
    glUniform1f(locW, width);
    glUniform1f(locH, height);
    glUniform4f(locC, r, g, b, a);

    int locShape = glGetUniformLocation(colorShader, "drawShape");
    glUniform1i(locShape, 2); // Trougao Levo (drawShape=2)

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUniform1i(locShape, 0);
    glBindVertexArray(0);
}

void drawTexturedQuad(float x, float y, float width, float height, unsigned int textureID, float u_scroll, float u_repeat) {
    glUseProgram(colorShader);

    // ⭐ Omogući teksturu
    glUniform1i(glGetUniformLocation(colorShader, "useTexture"), 1);

    // Uniformi za Poziciju/Veličinu
    int locX = glGetUniformLocation(colorShader, "positionX");
    int locY = glGetUniformLocation(colorShader, "positionY");
    int locW = glGetUniformLocation(colorShader, "width");
    int locH = glGetUniformLocation(colorShader, "height");
    int locC = glGetUniformLocation(colorShader, "color");
    int locShape = glGetUniformLocation(colorShader, "drawShape");

    glUniform1f(locX, x);
    glUniform1f(locY, y);
    glUniform1f(locW, width);
    glUniform1f(locH, height);

    // Crvena boja za EKG (A=1.0)
    glUniform4f(locC, 1.0f, 0.0f, 0.0f, 1.0f);

    glUniform1i(locShape, 0); // Uvek kvadrat za teksturu

    // Uniformi za Skrolovanje/Ponavljanje
    glUniform1f(glGetUniformLocation(colorShader, "u_scroll"), u_scroll);
    glUniform1f(glGetUniformLocation(colorShader, "u_repeat"), u_repeat);

    // Vezivanje teksture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(colorShader, "textureSampler"), 0);

    // Crtanje
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Čišćenje stanja (opciono, ali dobra praksa)
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(glGetUniformLocation(colorShader, "useTexture"), 0);
}

void drawWatchFrame() {
    const float R = 0.0f, G = 0.0f, B = 0.0f, A = 1.0f;

    // 1. Crtanje narukvice
    drawQuad(0.0f, 0.7f, 0.2f, 0.3f, R, G, B, A); // Gornji deo
    drawQuad(0.0f, -0.7f, 0.2f, 0.3f, R, G, B, A); // Donji deo

    // 2. Crtanje Osmerougla / Ekrana sata (Crni okvir)
    drawQuad(0.0f, 0.0f, FRAME_SIZE_X, FRAME_SIZE_Y, R, G, B, A);

    // Gornja ručka:
    drawQuad(0.0f, FRAME_SIZE_Y + 0.05f, FRAME_SIZE_X + 0.05f, 0.05f, R, G, B, A);
    // Donja ručka:
    drawQuad(0.0f, -FRAME_SIZE_Y - 0.05f, FRAME_SIZE_X + 0.05f, 0.05f, R, G, B, A);

    // 3. Iscrtavanje Bele Površine Ekrana unutar Okvira
    drawQuad(
        0.0f, 0.0f,
        FRAME_SIZE_X - 0.02f, FRAME_SIZE_Y - 0.02f,
        1.0f, 1.0f, 1.0f, 1.0f // Bela boja (Ekran)
    );
}