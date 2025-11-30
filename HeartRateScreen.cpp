#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib> 
#include <ctime>   
#include <iostream>
#include <string>

// =========================================================
// EXTERN DEKLARACIJE GLOBALNIH PROMENLJIVIH IZ main.cpp
// =========================================================

// Geometrija okvira (za pozicioniranje strelica)
extern const float FRAME_SIZE_X;
extern const float FRAME_SIZE_Y;
extern unsigned int ecgTextureID;
extern float ecgScrollOffset;
extern float ecgTextureRepeat;
extern float smoothedTextureRepeat;
extern int currentBPM;
extern double lastBPMUpdate;
extern double lastTimeUpdate;
extern bool isRunning;

extern void renderText(std::string text, float x, float y, float scale, float r, float g, float b, int windowHeight);

// Funkcije za crtanje (iz WatchFrame.cpp)
void drawWatchFrame();
void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTexturedQuad(float x, float y, float width, float height, unsigned int textureID, float u_scroll, float u_repeat);

void updateBPM(double currentTime) {
    if (isRunning) {
        // Logika trčanja će se implementirati kasnije. Za sada samo inkrementiramo.
    }
    else if (currentTime - lastBPMUpdate >= 1.0) {
        // 60 + (0 do 20) -> opseg [60, 80] za normalno stanje
        currentBPM = 60 + (std::rand() % 21);
        lastBPMUpdate = currentTime;
    }
}

void drawECG() {
    // 1. Ažuriranje Skrolovanja
    // Brzina skrolovanja je fiksna, ali je stopa ponavljanja povezana sa BPM-om!
    // Ako BPM raste, EKG se sužava (više otkucaja po širini).

    // Brzina horizontalne animacije
    const float SCROLL_SPEED = 0.005f;
    ecgScrollOffset -= SCROLL_SPEED;
    if (ecgScrollOffset < -1.0f) {
        ecgScrollOffset += 1.0f;
    }

    // Faktor ponavljanja teksture (Tekstura se ponavlja 2.0 puta normalno)
    // Povezujemo ponavljanje sa BPM-om: što je veći BPM, veći je repeat faktor (sužava se graf)
    // Normalno: 70 BPM -> 2.0 repeat. Za 80 BPM, može biti 2.5, za 60 -> 1.5.
    // Linearna veza: (currentBPM - 60) * 0.05 + 1.5
    ecgTextureRepeat = (currentBPM - 60.0f) * 0.05f + 1.5f;
    ecgTextureRepeat = std::max(1.5f, ecgTextureRepeat); // Min ponavljanje 1.5

    //za glatke tranzicije
    const float LERP_FACTOR = 0.01f;
    smoothedTextureRepeat += (ecgTextureRepeat - smoothedTextureRepeat) * LERP_FACTOR;

    // NDC Koordinate EKG kvadrata
    const float ECG_WIDTH = FRAME_SIZE_X - 0.12f;//alternativno 0.02
    const float ECG_HEIGHT = FRAME_SIZE_Y - 0.15f; // Malo uži
    const float ECG_X = 0.0f;
    const float ECG_Y = 0.0f;

    drawTexturedQuad(
        ECG_X, ECG_Y,
        ECG_WIDTH, ECG_HEIGHT,
        ecgTextureID,
        ecgScrollOffset,
        smoothedTextureRepeat
    );
}

void drawHeartRateScreen(GLFWwindow* window) {
    updateBPM(glfwGetTime());

    drawWatchFrame();

    drawECG();

    // 3. Prikaz BPM teksta Iznad EKG-a
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::string bpmStr = std::to_string(currentBPM) + " BPM";

    float scale = 1.0f;
    float fontSize = 48.0f * scale;

    // Aproksimacija širine teksta
    float textWidthEstimate = 0.6f * fontSize * bpmStr.length();

    // Pozicija X (centrirano)
    float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);

    // Pozicija Y (Iznad EKG-a) - NDC Y -0.2 (blizu gornje ivice ekrana sata)
    float posY_ndc = FRAME_SIZE_Y - 0.1f;
    float posY = (height / 2.0f) - (posY_ndc * (height / 2.0f)) + (fontSize / 2.0f);

    // Crtanje BPM teksta
    renderText(bpmStr, posX, posY, scale, 0.0f, 0.0f, 0.0f, height);

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