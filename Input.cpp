#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

// Globalne promenljive za stanje
enum ScreenState {
    TIME_SCREEN,
    HEART_RATE_SCREEN,
    BATTERY_SCREEN
};
extern ScreenState currentScreen;
extern const float FRAME_SIZE_X;

extern GLFWcursor* heartCursorDefault;
extern GLFWcursor* heartCursorActive;

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

//za prebacivanje iz fullscreen u windowed
void toggleFullscreen(GLFWwindow* window)
{
    isFullscreen = !isFullscreen;

    if (isFullscreen)
    {
        // cuvamo poziciju windowed
        glfwGetWindowPos(window, &windowPosX, &windowPosY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        // u fullscreen
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        // u windowed
        glfwSetWindowMonitor(window, NULL, windowPosX, windowPosY, windowedWidth, windowedHeight, 0);
    }
}

//za unos sa tastature
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
            // aktiviraj mod trčanja
            isRunning = true;
            if (currentBPM < 80) {
                currentBPM = 80;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            // deaktiviraj mod trčanja
            isRunning = false;
        }
    }
}

//pomocna funkcija za transformisanje lokacije kursora iz piksela u -1, 1 koordinate
void getCursorNDC(GLFWwindow* window, double xpos, double ypos, float& ndcX, float& ndcY)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    ndcX = (float)(xpos / width) * 2.0f - 1.0f;

    ndcY = 1.0f - (float)(ypos / height) * 2.0f;
}

//za unos sa misa
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // postavi teksturu za pritisnuti kursor
            if (heartCursorActive) {
                glfwSetCursor(window, heartCursorActive);
            }
        }
        else if (action == GLFW_RELEASE) {
            // vrati na podrazumevani kursor
            if (heartCursorDefault) {
                glfwSetCursor(window, heartCursorDefault);
            }
        }
    }

    //za navigaciju kroz ekrane
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float ndcX, ndcY;
        getCursorNDC(window, xpos, ypos, ndcX, ndcY);

        // zajednicke granice strelice
        const float ARROW_WIDTH = 0.03f;
        const float ARROW_HEIGHT = 0.04f;
        const float ARROW_Y_MIN = 0.0f - ARROW_HEIGHT;
        const float ARROW_Y_MAX = 0.0f + ARROW_HEIGHT;

        //pozicija desne strelice
        const float ARROW_POS_X_R = FRAME_SIZE_X - 0.08f;
        const float ARROW_X_MIN_R = ARROW_POS_X_R - ARROW_WIDTH;
        const float ARROW_X_MAX_R = ARROW_POS_X_R + ARROW_WIDTH;

        //pozicija leve strelice
        const float ARROW_POS_X_L = -FRAME_SIZE_X + 0.08f;
        const float ARROW_X_MIN_L = ARROW_POS_X_L - ARROW_WIDTH;
        const float ARROW_X_MAX_L = ARROW_POS_X_L + ARROW_WIDTH;

        //provera sudara i prebacivanje ekrana
        if (ndcY >= ARROW_Y_MIN && ndcY <= ARROW_Y_MAX)
        {
            switch (currentScreen) {
            case TIME_SCREEN:
                //desno (TIME -> HEART_RATE)
                if (ndcX >= ARROW_X_MIN_R && ndcX <= ARROW_X_MAX_R)
                {
                    currentScreen = HEART_RATE_SCREEN;
                    std::cout << ">>> Prebacivanje na: HEART_RATE_SCREEN" << std::endl;
                }
                break;

            case HEART_RATE_SCREEN:
                //desno (HEART_RATE -> BATTERY)
                if (ndcX >= ARROW_X_MIN_R && ndcX <= ARROW_X_MAX_R)
                {
                    currentScreen = BATTERY_SCREEN;
                    std::cout << ">>> Prebacivanje na: BATTERY_SCREEN" << std::endl;
                }
                //levo (HEART_RATE -> TIME)
                else if (ndcX >= ARROW_X_MIN_L && ndcX <= ARROW_X_MAX_L)
                {
                    currentScreen = TIME_SCREEN;
                    std::cout << ">>> Prebacivanje na: TIME_SCREEN" << std::endl;
                }
                break;

            case BATTERY_SCREEN:
                //levo (BATTERY -> HEART_RATE)
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