#include <GL/glew.h>
#include <GLFW/glfw3.h>

// =========================================================
// EXTERN DEKLARACIJE GLOBALNIH PROMENLJIVIH IZ main.cpp
// =========================================================

// Geometrija okvira (za pozicioniranje strelica)
extern const float FRAME_SIZE_X;

// Funkcije za crtanje (iz WatchFrame.cpp)
void drawWatchFrame();
void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);

// =========================================================
// DEFINICIJA FUNKCIJE
// =========================================================

void drawHeartRateScreen(GLFWwindow* window) {
    drawWatchFrame();

    const float ARROW_POS_Y = 0.0f;
    const float ARROW_WIDTH = 0.03f;
    const float ARROW_HEIGHT = 0.04f;
    const float ARROW_POS_X_R = FRAME_SIZE_X - 0.08f;
    const float ARROW_POS_X_L = -FRAME_SIZE_X + 0.08f;

    // Desna strelica (za BATTERY_SCREEN)
    drawTriangleRight(
        ARROW_POS_X_R, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // Leva strelica (za TIME_SCREEN)
    drawTriangleLeft(
        ARROW_POS_X_L, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}