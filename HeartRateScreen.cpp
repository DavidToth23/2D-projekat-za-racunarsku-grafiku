#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib> 
#include <ctime>   
#include <iostream>
#include <string>

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

// Funkcije za crtanje
void drawWatchFrame();
void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTexturedQuad(float x, float y, float width, float height, unsigned int textureID, float u_scroll, float u_repeat);

//logika izmene BPM
void updateBPM(double currentTime) {
    //logika za trcanje
    if (isRunning) {
        if (currentTime - lastRunnerUpdate >= 0.5) { //menjamo BPM svakih 0.5 sekundi
            if (currentBPM < 250) {
                currentBPM = currentBPM + 3 + (std::rand() % 3); //povecavamo BPM za 3 do 5
            }
            lastRunnerUpdate = currentTime;
        }
        //osiguravamo minimalan BPM da je iznad 80
        currentBPM = std::max(80, currentBPM);

    }
    else if (currentTime - lastBPMUpdate >= 1.0) {
        //logika oporavka od trcanja
        if (currentBPM > 80) {
            currentBPM = std::max(60, currentBPM - 5 - (std::rand() % 6)); //smanjujemo BPM za 5 do 10
        }
        else {
            currentBPM = 60 + (std::rand() % 21); // BPM u normalnom stanju od 60 do 80
        }
        lastBPMUpdate = currentTime;
    }
}

// iscrtavanje ekg-a
void drawECG() {
    //brzina horizontalne animacije(skrolovanja)
    const float SCROLL_SPEED = 0.007f;
    ecgScrollOffset -= SCROLL_SPEED;
    if (ecgScrollOffset < -1.0f) {
        ecgScrollOffset += 1.0f;
    }

    //logika mapiranja vrednosti BPM-a na ponavljanje teksture
    ecgTextureRepeat = (currentBPM - 60.0f) * 0.02f + 1.2f;
    ecgTextureRepeat = std::max(1.2f, ecgTextureRepeat); //min ponavljanje 1.2

    //za glatke tranzicije(linearna interpolacija)
    const float LERP_FACTOR = 0.01f;
    smoothedTextureRepeat += (ecgTextureRepeat - smoothedTextureRepeat) * LERP_FACTOR;

    //koordinate ekg kvadra u NDC koordinatnom sistemu
    const float ECG_WIDTH = FRAME_SIZE_X - 0.12f;
    const float ECG_HEIGHT = FRAME_SIZE_Y - 0.15f;
    const float ECG_X = 0.0f;
    const float ECG_Y = 0.0f;

    //iscrtavanje
    drawTexturedQuad(
        ECG_X, ECG_Y,
        ECG_WIDTH, ECG_HEIGHT,
        ecgTextureID,
        ecgScrollOffset,
        smoothedTextureRepeat
    );
}

// iscrtavanje ekrana
void drawHeartRateScreen(GLFWwindow* window) {
    updateBPM(glfwGetTime());

    drawWatchFrame();

    drawECG();

    //prikaz vrednosti BPM iznad EKG grafa
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::string bpmStr = std::to_string(currentBPM) + " BPM";

    float scale = 1.0f;
    float fontSize = 48.0f * scale;
    float textWidthEstimate = 0.6f * fontSize * bpmStr.length();

    float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);
    float posY_ndc = FRAME_SIZE_Y - 0.1f;
    float posY = (height / 2.0f) - (posY_ndc * (height / 2.0f)) + (fontSize / 2.0f);

    //iscrtavanje BPM teksta
    renderText(bpmStr, posX, posY, scale, 0.0f, 0.0f, 0.0f, height);

    const float ARROW_POS_Y = 0.0f;
    const float ARROW_WIDTH = 0.03f;
    const float ARROW_HEIGHT = 0.04f;
    const float ARROW_POS_X_R = FRAME_SIZE_X - 0.08f;
    const float ARROW_POS_X_L = -FRAME_SIZE_X + 0.08f;

    //desna strelica
    drawTriangleRight(
        ARROW_POS_X_R, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    //leva strelica
    drawTriangleLeft(
        ARROW_POS_X_L, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    //upozorenje za previsok BPM
    if (currentBPM >= 200) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        std::string warningText = "STOP AND REST!";
        float scale = 1.0f;

        //crtanje crvenog kvadrata preko cele povrsine ekrana sata
        drawQuad(
            0.0f, 0.0f,
            FRAME_SIZE_X * 0.95f, FRAME_SIZE_Y * 0.95f,
            1.0f, 0.0f, 0.0f, 0.5f //crvena boja 50% transparentnost
        );

        float textWidthEstimate = 0.6f * (48.0f * scale) * warningText.length();
        float fontSize = 48.0f * scale;

        float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);
        float posY = (height / 1.75f) + (fontSize / 2.0f);

        renderText(warningText, posX, posY, scale, 0.0f, 0.0f, 0.0f, height);
    }
}