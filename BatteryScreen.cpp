#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <algorithm>

// Geometrija okvira (za pozicioniranje strelica)
extern const float FRAME_SIZE_X;
extern const float FRAME_SIZE_Y;
extern int batteryLevel;
extern double lastBatteryUpdate;

// Funkcije za crtanje (iz WatchFrame.cpp)
void drawWatchFrame();
void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);
extern void renderText(std::string text, float x, float y, float scale, float r, float g, float b, int windowHeight);

void updateBattery(double currentTime) {
    // Provera da li je prošlo 10 sekundi od poslednjeg ažuriranja
    if (currentTime - lastBatteryUpdate >= 0.2) {
        if (batteryLevel > 0) {
            // Smanji bateriju za 1%
            batteryLevel = std::max(0, batteryLevel - 1);
        }
        lastBatteryUpdate = currentTime; // Resetovanje brojača
    }
}

void drawBatteryScreen(GLFWwindow* window) {
    updateBattery(glfwGetTime());

    drawWatchFrame();

    const float BATT_WIDTH = FRAME_SIZE_X * 0.6f;   // Široka baterija
    const float BATT_HEIGHT = FRAME_SIZE_Y * 0.3f;  // Visina baterije
    const float BATT_X = 0.0f;                      // Centar baterije
    const float BATT_Y = 0.0f;                      // Centar baterije
    const float BATT_THICKNESS = 0.01f;            // Debljina okvira

    drawQuad(
        BATT_X, BATT_Y,
        BATT_WIDTH, BATT_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f // Crna boja
    );
    // b) Unutrašnji beli/providni pravougaonik (stvara se efekat okvira)
    drawQuad(
        BATT_X, BATT_Y,
        BATT_WIDTH - BATT_THICKNESS, BATT_HEIGHT - BATT_THICKNESS,
        1.0f, 1.0f, 1.0f, 1.0f // Bela pozadina
    );

    // c) Vrh baterije (mali kvadrat desno)
    const float BATT_CAP_WIDTH = 0.008f;
    const float BATT_CAP_HEIGHT = BATT_HEIGHT * 0.4f;
    const float BATT_CAP_X = BATT_WIDTH / 1.0f + BATT_CAP_WIDTH / 2.0f; // Nalazi se na desnoj ivici glavnog tela

    drawQuad(
        BATT_CAP_X, BATT_Y,
        BATT_CAP_WIDTH, BATT_CAP_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f // Crna boja
    );

    float r = 0.0f, g = 1.0f, b = 0.0f; // Podrazumevana: Zelena
    if (batteryLevel <= 20 && batteryLevel > 10) {
        r = 1.0f; g = 1.0f; b = 0.0f; // Žuta (<= 20%)
    }
    else if (batteryLevel <= 10) {
        r = 1.0f; g = 0.0f; b = 0.0f; // Crvena (<= 10%)
    }

    const float FILL_MAX_WIDTH = BATT_WIDTH - BATT_THICKNESS;
    const float FILL_HEIGHT = BATT_HEIGHT - BATT_THICKNESS;

    // Trenutna širina punjenja
    float currentFillWidth = FILL_MAX_WIDTH * ((float)batteryLevel / 100.0f);

    // X pozicija punjenja
    // Punjenje mora biti "zalepljeno" na desnu ivicu: 
    // Max desna ivica: (BATT_X + FILL_MAX_WIDTH / 2.0f)
    // Centar punjenja: Max desna ivica - (currentFillWidth / 2.0f)
    float fillStartX = BATT_X - FILL_MAX_WIDTH / 1.0f; // Najleva tačka unutrašnjeg prostora
    float fillX = fillStartX + currentFillWidth / 1.0f;

    if (currentFillWidth > 0.0f) {
        drawQuad(
            fillX, BATT_Y,
            currentFillWidth, FILL_HEIGHT,
            r, g, b, 1.0f
        );
    }

    // 5. Crtanje procenta iznad baterije
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::string battStr = std::to_string(batteryLevel) + "%";

    // Pozicija Y (Iznad baterije) - NDC Y 0.2 (ili slicno)
    float posY_ndc = -BATT_HEIGHT / 0.5f + 0.1f;
    float scale = 1.0f;

    // Aproksimacija širine teksta za centriranje
    // Koristimo 0.6f * (visina fonta * scale) * duzina
    float fontSize = 48.0f * scale;
    float textWidthEstimate = 0.6f * fontSize * battStr.length();

    float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);
    float posY = (height / 2.0f) - (posY_ndc * (height / 2.0f)) + (fontSize / 2.0f);

    // Crno ili belo (ako je baterija prazna)
    float textR = 0.0f, textG = 0.0f, textB = 0.0f;
    if (batteryLevel <= 10) {
        textR = 1.0f; textG = 0.0f; textB = 0.0f; // Crveni tekst za nisku bateriju
    }

    renderText(battStr, posX, posY, scale, textR, textG, textB, height);

    const float ARROW_POS_Y = 0.0f;
    const float ARROW_WIDTH = 0.03f;
    const float ARROW_HEIGHT = 0.04f;
    const float ARROW_POS_X_L = -FRAME_SIZE_X + 0.08f;

    // Leva strelica (za HEART_RATE_SCREEN)
    drawTriangleLeft(
        ARROW_POS_X_L, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}