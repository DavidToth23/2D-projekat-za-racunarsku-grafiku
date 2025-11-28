#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include "Util.h"

// Global state
bool isFullscreen = true;
int windowedWidth = 1280;
int windowedHeight = 720;
int windowPosX = 100;
int windowPosY = 100;

GLFWmonitor* monitor;
const GLFWvidmode* mode;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

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
}

int main()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get monitor info
    monitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(monitor);

    // Create fullscreen window
    GLFWwindow* window = glfwCreateWindow(
        mode->width,
        mode->height,
        "Pametni sat - David Toth",
        monitor,
        NULL
    );

    if (!window)
        return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // GLEW init
    if (glewInit() != GLEW_OK)
        return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    unsigned int colorShader = createShader("color.vert", "color.frag");

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f);

    // MAIN LOOP
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
