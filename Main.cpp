#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <algorithm>
#include <iostream>
#include "Util.h"
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

//deklaracije iz input.cpp(za upravljanje logikom sa tastature i misa)
void toggleFullscreen(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void getCursorNDC(GLFWwindow* window, double xpos, double ypos, float& ndcX, float& ndcY);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

//deklaracije iz watchframe.cpp
void initQuad();
void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleRight(float x, float y, float width, float height, float r, float g, float b, float a);
void drawTriangleLeft(float x, float y, float width, float height, float r, float g, float b, float a);
void drawWatchFrame();

//deklaracije iz timescreen.cpp
void drawTimeScreen(GLFWwindow* window);

// deklaracije heartratescreen.cpp
void drawHeartRateScreen(GLFWwindow* window);

// Prototipovi iz BatteryScreen.cpp
void drawBatteryScreen(GLFWwindow* window);

//Freetype globals
unsigned int textShader = 0;
// Map za skladištenje karaktera (glifova)
struct Character {
    unsigned int TextureID; // ID teksture glifa
    // dimenzije glifa
    int Width;
    int Height;
    // pomak sa bazne linije
    int BearingX;
    int BearingY;
    // pomak na sledeći glif
    long Advance;
};

std::map<char, Character> Characters;

// Globalni VAO/VBO za tekst
unsigned int textVAO = 0;
unsigned int textVBO = 0;

// Dodajemo enum za stanje ekrana (prikaz)
enum ScreenState {
    TIME_SCREEN,
    HEART_RATE_SCREEN,
    BATTERY_SCREEN
};

// Global state
bool isFullscreen = true;
int windowedWidth = 1280;
int windowedHeight = 720;
int windowPosX = 100;
int windowPosY = 100;
unsigned int rectShader = 0;
unsigned int colorShader = 0;
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
ScreenState currentScreen = TIME_SCREEN;

// === NOVE VARIJABLE ZA HEART RATE SCREEN ===
unsigned int ecgTextureID = 0; // ID teksture za EKG liniju
float ecgScrollOffset = 0.0f;  // Trenutni pomak X (za animaciju)
float ecgTextureRepeat = 2.0f; // Faktor ponavljanja teksture (za BPM vizuelizaciju)
extern float smoothedTextureRepeat = 2.0f; //Vrednost koja se koristi za crtanje
int currentBPM = 70;           // Trenutna BPM vrednost
double lastBPMUpdate = 0.0;    // Vreme poslednjeg ažuriranja BPM-a
double lastRunnerUpdate = 0.0; // Vreme poslednjeg ažuriranja za trčanje
bool isRunning = false;        // Da li je taster D pritisnut (Runner mod)

int hours = 10;   // Početno vreme, npr. 10:30:00
int minutes = 30;
int seconds = 0;
double lastTimeUpdate = 0.0;

extern const float FRAME_SIZE_X = 0.35f; // Polu-širina ekrana sata
extern const float FRAME_SIZE_Y = 0.40f; // Polu-visina ekrana sata

GLFWmonitor* monitor;
const GLFWvidmode* mode;

// Funkcija za inicijalizaciju FreeType-a i učitavanje glifova
int initTextRenderer(int windowWidth, int windowHeight) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // Učitaj font, npr. Arial.ttf. Mora postojati u folderu!
    FT_Face face;
    if (FT_New_Face(ft, "arial.ttf", 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        FT_Done_FreeType(ft);
        return -1;
    }

    // Postavljanje veličine fonta (širina, visina)
    // 0 za širinu znači da će širina biti dinamički određena. 
    FT_Set_Pixel_Sizes(face, 0, 48); // Visina 48 piksela

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // onemogući poravnanje bajtova

    // Učitavanje prvih 128 ASCII karaktera
    for (unsigned char c = 0; c < 128; c++)
    {
        // Učitavanje glifa
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph " << c << std::endl;
            continue;
        }

        // Generisanje teksture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED, // Jednobojni glif (samo crvena komponenta se koristi za alfa)
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Parametri teksture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Skladištenje podataka karaktera
        Character character = {
            texture,
            (int)face->glyph->bitmap.width,
            (int)face->glyph->bitmap.rows,
            (int)face->glyph->bitmap_left,
            (int)face->glyph->bitmap_top,
            (long)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    // Čišćenje FreeType resursa
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Kreiramo VAO/VBO za crtanje teksta (sličan kao quadVAO/VBO, ali se podaci menjaju)
    // Nećemo koristiti postojeći quadVAO jer se podaci teksture često menjaju.
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); // 6 verteksa * 4 float (x,y,u,v)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return 0; // Uspešno inicijalizovano
}

void renderText(std::string text, float x, float y, float scale, float r, float g, float b, int windowHeight)
{
    glUseProgram(textShader);

    // Pošto FreeType radi u koordinatama piksela, moramo da podesimo ortografsku matricu.
    // Matrica transformiše piksel koordinate (npr. 0 do 1280) u NDC koordinate.
    // Ovde koristimo height za y-os (top=0, bottom=windowHeight)
    float projection[16]; // Matrica 4x4
    // Kreiraj orto matricu (jednostavna implementacija za 2D rendering)
    float right = (float)windowHeight * ((float)windowedWidth / windowedHeight);

    // Jednostavna ortografska projekcija (bez z)
    // Moguće je da ćeš morati da ovu matricu promeniš u zavisnosti od tvoje implementacije.
    // Za sada, koristićemo jednostavnu 2D projekciju za prozor.

    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

    // Obično FreeType Rendering koristi top-left ugao (0, 0) kao početak
    // Da bismo ga koristili, moramo napraviti orto matricu koja odgovara: 
    // left=0, right=width, bottom=0, top=height (ili obrnuto za y-koordinate)

    // Projekcija: 0 - width na X osi, height - 0 na Y osi (obrnut Y)
    // Ovo omogućava da y=0 bude gornja ivica prozora
    float ortho[16] = {
        2.0f / width, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / height, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };

    glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, ortho);
    glUniform3f(glGetUniformLocation(textShader, "textColor"), r, g, b);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iteracija kroz karaktere
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        // Izračunavanje pozicije i veličine kvadra za glif
        float xpos = x + ch.BearingX * scale;
        float ypos = y + (ch.BearingY - ch.Height) * scale;

        float w = ch.Width * scale;
        float h = ch.Height * scale;

        // Vertikalni podaci kvadra (pozicija i UV koordinate)
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Crtanje glifa
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Ažuriranje VBO memorije
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Crtanje
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Pomeranje X pozicije za sledeći karakter (pomak + pomak glifa)
        x += (ch.Advance >> 6) * scale; // Bitshift za 6 (64) zbog FreeType konvencije
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void updateTime(double currentTime) {
    if (currentTime - lastTimeUpdate >= 1.0) { // Prošlo je 1.0 sekunda
        seconds++;
        lastTimeUpdate = currentTime; // Resetovanje brojača

        if (seconds >= 60) {
            seconds = 0;
            minutes++;

            if (minutes >= 60) {
                minutes = 0;
                hours++;

                if (hours >= 24) {
                    hours = 0; // Reset na 00:00:00
                }
            }
        }
    }
}

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // GLEW init
    if (glewInit() != GLEW_OK)
        return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    rectShader = createShader("rect.vert", "rect.frag");
    colorShader = createShader("color.vert", "color.frag");
    textShader = createShader("text.vert", "text.frag");

    if (initTextRenderer(mode->width, mode->height) != 0)
        return endProgram("FreeType inicijalizacija nije uspela.");

    // ⭐ Učitavanje EKG Teksture
    ecgTextureID = loadImageToTexture("ecg_line.png"); // Pretpostavljamo da imate ecg_line.png
    if (ecgTextureID == 0) {
        std::cerr << "ERROR: Failed to load ECG texture." << std::endl;
        // Opciono: učitati placeholder
    }

    // Seedovanje za random BPM
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    initQuad();

    lastTimeUpdate = glfwGetTime();

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f);

    // MAIN LOOP
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        // Ažuriranje stanja (Logika)
        updateTime(currentTime);

        glClear(GL_COLOR_BUFFER_BIT);

        switch (currentScreen) {
        case TIME_SCREEN:
            drawTimeScreen(window);
            break;
        case HEART_RATE_SCREEN:
            drawHeartRateScreen(window);
            break;
        case BATTERY_SCREEN:
            drawBatteryScreen(window);
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &textVBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteProgram(textShader);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
