#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <algorithm>

//geometrija okvira (za pozicioniranje strelica)
extern const float FRAME_SIZE_X;
extern const float FRAME_SIZE_Y;
extern int batteryLevel;
extern double lastBatteryUpdate;

// funkcije za crtanje (iz WatchFrame.cpp)
void drawWatchFrame();
void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);
extern void renderText(std::string text, float x, float y, float scale, float r, float g, float b, int windowHeight);

//logika za praznjenje baterije
void updateBattery(double currentTime) {
    //smanjujemo za svakih 10 sekundi
    if (currentTime - lastBatteryUpdate >= 10.0) {
        if (batteryLevel > 0) {
            //smanji bateriju za 1%
            batteryLevel = std::max(0, batteryLevel - 1);
        }
        lastBatteryUpdate = currentTime;
    }
}

//iscrtavanje baterije
void drawBatteryScreen(GLFWwindow* window) {
    drawWatchFrame();

    const float BATT_WIDTH = FRAME_SIZE_X * 0.6f;   
    const float BATT_HEIGHT = FRAME_SIZE_Y * 0.3f;  
    const float BATT_X = 0.0f;                      
    const float BATT_Y = 0.0f;              
    const float BATT_THICKNESS = 0.01f;

    //crni kvadrat
    drawQuad(
        BATT_X, BATT_Y,
        BATT_WIDTH, BATT_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f //crna boja
    );

    //malo manji beli kvadrat(dobijamo okvir)
    drawQuad(
        BATT_X, BATT_Y,
        BATT_WIDTH - BATT_THICKNESS, BATT_HEIGHT - BATT_THICKNESS,
        1.0f, 1.0f, 1.0f, 1.0f //bela boja
    );

    const float BATT_CAP_WIDTH = 0.008f;
    const float BATT_CAP_HEIGHT = BATT_HEIGHT * 0.4f;
    const float BATT_CAP_X = BATT_WIDTH / 1.0f + BATT_CAP_WIDTH / 2.0f;
    
    //vrh baterije
    drawQuad(
        BATT_CAP_X, BATT_Y,
        BATT_CAP_WIDTH, BATT_CAP_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f //crna boja
    );

    //boja punjenja baterije
    float r = 0.0f, g = 1.0f, b = 0.0f; //zelena boja
    if (batteryLevel <= 20 && batteryLevel > 10) {
        r = 1.0f; g = 1.0f; b = 0.0f; //zuta boja <20%
    }
    else if (batteryLevel <= 10) {
        r = 1.0f; g = 0.0f; b = 0.0f; //crvena boja <10%
    }

    const float FILL_MAX_WIDTH = BATT_WIDTH - BATT_THICKNESS;
    const float FILL_HEIGHT = BATT_HEIGHT - BATT_THICKNESS;

    //logika za smanjivanje punjenja baterije
    float currentFillWidth = FILL_MAX_WIDTH * ((float)batteryLevel / 100.0f);
    float fillStartX = BATT_X - FILL_MAX_WIDTH / 1.0f;
    float fillX = fillStartX + currentFillWidth / 1.0f;

    if (currentFillWidth > 0.0f) {
        drawQuad(
            fillX, BATT_Y,
            currentFillWidth, FILL_HEIGHT,
            r, g, b, 1.0f
        );
    }

    //iscrtavanje procenta iznad baterije
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    std::string battStr = std::to_string(batteryLevel) + "%";

    float posY_ndc = -BATT_HEIGHT / 0.5f + 0.1f;
    float scale = 1.0f;
    float fontSize = 48.0f * scale;
    float textWidthEstimate = 0.6f * fontSize * battStr.length();

    float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);
    float posY = (height / 2.0f) - (posY_ndc * (height / 2.0f)) + (fontSize / 2.0f);

    float textR = 0.0f, textG = 0.0f, textB = 0.0f;

    renderText(battStr, posX, posY, scale, textR, textG, textB, height);
    
    //leva strelica
    const float ARROW_POS_Y = 0.0f;
    const float ARROW_WIDTH = 0.03f;
    const float ARROW_HEIGHT = 0.04f;
    const float ARROW_POS_X_L = -FRAME_SIZE_X + 0.08f;

    drawTriangleLeft(
        ARROW_POS_X_L, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}