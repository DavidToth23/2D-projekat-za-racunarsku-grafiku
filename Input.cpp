#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

// =========================================================
// EXTERN DEKLARACIJE GLOBALNIH PROMENLJIVIH IZ main.cpp
// =========================================================

// Globalne promenljive za stanje
enum ScreenState {
    TIME_SCREEN,
    HEART_RATE_SCREEN,
    BATTERY_SCREEN
};
extern ScreenState currentScreen;
extern const float FRAME_SIZE_X;

// Globalne promenljive za fullscreen
extern bool isFullscreen;
extern int windowedWidth;
extern int windowedHeight;
extern int windowPosX;
extern int windowPosY;
extern GLFWmonitor* monitor;
extern const GLFWvidmode* mode;

extern bool isRunning;
extern int currentBPM;

void toggleFullscreen(GLFWwindow* window)
{
    isFullscreen = !isFullscreen;

    if (isFullscreen)
    {
        // Save windowed position & size
        glfwGetWindowPos(window, &windowPosX, &windowPosY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        // Switch to fullscreen
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        // Switch to windowed
        glfwSetWindowMonitor(window, NULL, windowPosX, windowPosY, windowedWidth, windowedHeight, 0);
    }
}

/**
 * Callback funkcija za obradu unosa sa tastature.
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }

        if (key == GLFW_KEY_F11) {
            toggleFullscreen(window);
        }
    }

    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)
        {
            // Aktiviraj mod trčanja
            isRunning = true;
            if (currentBPM < 80) {
                currentBPM = 80;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            // Deaktiviraj mod trčanja
            isRunning = false;
        }
    }
}

/**
 * Pretvara koordinate kursora (pikseli) u NDC koordinate (-1.0 do 1.0).
 */
void getCursorNDC(GLFWwindow* window, double xpos, double ypos, float& ndcX, float& ndcY)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // X: (x_piksela / width) * 2 - 1
    ndcX = (float)(xpos / width) * 2.0f - 1.0f;

    // Y: 1 - (y_piksela / height) * 2
    // Ekran koordinate idu od gore (0) dole (max), NDC idu od dole (-1) gore (1)
    ndcY = 1.0f - (float)(ypos / height) * 2.0f;
}

/**
 * Callback funkcija za obradu klika miša i promenu ekrana.
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float ndcX, ndcY;
        getCursorNDC(window, xpos, ypos, ndcX, ndcY);

        // Zajedničke granice strelice (NDC centar je (0,0))
        const float ARROW_WIDTH = 0.03f;
        const float ARROW_HEIGHT = 0.04f;
        const float ARROW_Y_MIN = 0.0f - ARROW_HEIGHT;
        const float ARROW_Y_MAX = 0.0f + ARROW_HEIGHT;

        // Desna strelica pozicija
        const float ARROW_POS_X_R = FRAME_SIZE_X - 0.08f;
        const float ARROW_X_MIN_R = ARROW_POS_X_R - ARROW_WIDTH;
        const float ARROW_X_MAX_R = ARROW_POS_X_R + ARROW_WIDTH;

        // Leva strelica pozicija
        const float ARROW_POS_X_L = -FRAME_SIZE_X + 0.08f;
        const float ARROW_X_MIN_L = ARROW_POS_X_L - ARROW_WIDTH;
        const float ARROW_X_MAX_L = ARROW_POS_X_L + ARROW_WIDTH;

        // Provera sudara i prebacivanje ekrana
        if (ndcY >= ARROW_Y_MIN && ndcY <= ARROW_Y_MAX) // Samo proveri da li je Y u zoni strelica
        {
            switch (currentScreen) {
            case TIME_SCREEN:
                // Strelica za desno (TIME -> HEART_RATE)
                if (ndcX >= ARROW_X_MIN_R && ndcX <= ARROW_X_MAX_R)
                {
                    currentScreen = HEART_RATE_SCREEN;
                    std::cout << ">>> Prebacivanje na: HEART_RATE_SCREEN" << std::endl;
                }
                break;

            case HEART_RATE_SCREEN:
                // Strelica za desno (HEART_RATE -> BATTERY)
                if (ndcX >= ARROW_X_MIN_R && ndcX <= ARROW_X_MAX_R)
                {
                    currentScreen = BATTERY_SCREEN;
                    std::cout << ">>> Prebacivanje na: BATTERY_SCREEN" << std::endl;
                }
                // Strelica za levo (HEART_RATE -> TIME)
                else if (ndcX >= ARROW_X_MIN_L && ndcX <= ARROW_X_MAX_L)
                {
                    currentScreen = TIME_SCREEN;
                    std::cout << ">>> Prebacivanje na: TIME_SCREEN" << std::endl;
                }
                break;

            case BATTERY_SCREEN:
                // Strelica za levo (BATTERY -> HEART_RATE)
                if (ndcX >= ARROW_X_MIN_L && ndcX <= ARROW_X_MAX_L)
                {
                    currentScreen = HEART_RATE_SCREEN;
                    std::cout << ">>> Prebacivanje na: HEART_RATE_SCREEN" << std::endl;
                }
                break;
            }
        }
    }
}