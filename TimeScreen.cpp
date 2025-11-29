#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

// =========================================================
// EXTERN DEKLARACIJE GLOBALNIH PROMENLJIVIH IZ main.cpp
// =========================================================

// Vreme
extern int hours;
extern int minutes;
extern int seconds;

// Geometrija okvira (za pozicioniranje strelica)
extern const float FRAME_SIZE_X;

// Funkcije za crtanje i tekst (iz WatchFrame.cpp i main.cpp)
void drawWatchFrame();
void renderText(std::string text, float x, float y, float scale, float r, float g, float b, int windowHeight);
void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a);

// =========================================================
// DEFINICIJA FUNKCIJE
// =========================================================

void drawTimeScreen(GLFWwindow* window) {
    drawWatchFrame();

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Formatiranje vremena
    std::string timeStr =
        (hours < 10 ? "0" : "") + std::to_string(hours) + ":" +
        (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
        (seconds < 10 ? "0" : "") + std::to_string(seconds);

    // --- Pozicioniranje teksta ---
    float scale = 1.0f;
    float fontSize = 48.0f * scale;
    float textWidthEstimate = 0.6f * fontSize * timeStr.length();

    float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);
    float posY = (height / 2.0f) + (fontSize / 2.0f) - 5;

    // Crtanje crnog vremena
    renderText(timeStr, posX, posY, scale, 0.0f, 0.0f, 0.0f, height);

    // Desna strelica (za HEART_RATE_SCREEN)
    const float ARROW_POS_X = FRAME_SIZE_X - 0.08f;
    const float ARROW_POS_Y = 0.0f;
    const float ARROW_WIDTH = 0.03f;
    const float ARROW_HEIGHT = 0.04f;

    drawTriangleRight(
        ARROW_POS_X, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f // Crna boja
    );
}