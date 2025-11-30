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
extern double lastRunnerUpdate;

extern void renderText(std::string text, float x, float y, float scale, float r, float g, float b, int windowHeight);

// Funkcije za crtanje (iz WatchFrame.cpp)
void drawWatchFrame();
void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTexturedQuad(float x, float y, float width, float height, unsigned int textureID, float u_scroll, float u_repeat);

void updateBPM(double currentTime) {
    if (isRunning) {
        if (currentTime - lastRunnerUpdate >= 0.5) { // Povećava BPM svake 0.5 sekunde
            if (currentBPM < 250) {
                currentBPM = currentBPM + 3 + (std::rand() % 3); // Povecavamo BPM za 3 do 5
            }
            lastRunnerUpdate = currentTime;
        }
        // Osiguravamo da se BPM ne spušta ispod 80 dok se trči
        currentBPM = std::max(80, currentBPM);

    }
    else if (currentTime - lastBPMUpdate >= 1.0) {
        if (currentBPM > 80) {
            // Ako je BPM bio visok (trčanje), polako ga spuštamo (oporavak)
            // Spuštanje za 5 do 10
            currentBPM = std::max(60, currentBPM - 5 - (std::rand() % 6));
        }
        else {
            // Normalni opseg BPM [60, 80] za mirno stanje
            currentBPM = 60 + (std::rand() % 21);
        }
        lastBPMUpdate = currentTime;
    }
}

void drawECG() {
    // Brzina horizontalne animacije
    const float SCROLL_SPEED = 0.007f;
    ecgScrollOffset -= SCROLL_SPEED;
    if (ecgScrollOffset < -1.0f) {
        ecgScrollOffset += 1.0f;
    }

    // Faktor ponavljanja teksture (Tekstura se ponavlja 2.0 puta normalno)
    // Povezujemo ponavljanje sa BPM-om: što je veći BPM, veći je repeat faktor (sužava se graf)
    // Normalno: 70 BPM -> 2.0 repeat. Za 80 BPM, može biti 2.5, za 60 -> 1.5.
    // Linearna veza: (currentBPM - 60) * 0.05 + 1.5
    ecgTextureRepeat = (currentBPM - 60.0f) * 0.02f + 1.2f;
    ecgTextureRepeat = std::max(1.2f, ecgTextureRepeat); // Min ponavljanje 1.5

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

    if (currentBPM >= 200) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        std::string warningText = "STOP AND REST!";
        float scale = 1.0f; // Veći font

        // Crtanje crvenog kvadra preko cele površine ekrana sata
        // Koristimo FRAME_SIZE_X i Y za dimenzije
        drawQuad(
            0.0f, 0.0f,
            FRAME_SIZE_X * 0.95f, FRAME_SIZE_Y * 0.95f,
            1.0f, 0.0f, 0.0f, 0.5f // Crvena boja, 70% transparentnost
        );

        // Gruba procena širine teksta za centriranje
        // (Ovo je aproksimacija jer niste poslali funkciju za precizno merenje teksta)
        float textWidthEstimate = 0.6f * (48.0f * scale) * warningText.length();
        float fontSize = 48.0f * scale;

        // X pozicija (Centrirano na ekranu prozora)
        float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);
        // Y pozicija (Centrirano na ekranu prozora)
        float posY = (height / 1.75f) + (fontSize / 2.0f);

        // Crno upozorenje iznad crvene pozadine
        renderText(warningText, posX, posY, scale, 0.0f, 0.0f, 0.0f, height);
    }
}