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

int hours = 10;   // Početno vreme, npr. 10:30:00
int minutes = 30;
int seconds = 0;
double lastTimeUpdate = 0.0;

const float FRAME_SIZE_X = 0.35f; // Polu-širina ekrana sata
const float FRAME_SIZE_Y = 0.40f; // Polu-visina ekrana sata

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

void drawQuad(float x, float y, float width, float height, float r, float g, float b, float a) {
    glUseProgram(colorShader);

    // Morate poslati uniform varijable u shader da transformišete 
    // NDC kvadrat (-1 do 1) u željeni položaj i veličinu na ekranu.

    // Kreiranje Model Matrice (Transformacija)
    // Koristimo jednostavan 2D pristup bez matrica za sada, šaljemo 
    // samo poziciju i veličinu kao uniforme.

    int locX = glGetUniformLocation(colorShader, "positionX");
    int locY = glGetUniformLocation(colorShader, "positionY");
    int locW = glGetUniformLocation(colorShader, "width");
    int locH = glGetUniformLocation(colorShader, "height");
    int locC = glGetUniformLocation(colorShader, "color");

    // Postavljanje uniformi
    glUniform1f(locX, x);
    glUniform1f(locY, y);
    glUniform1f(locW, width);
    glUniform1f(locH, height);
    glUniform4f(locC, r, g, b, a);

    // Crtanje
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void drawWatchFrame() {
    // Boja: Crna (RGB: 0, 0, 0, Alpha: 1)
    const float R = 0.0f, G = 0.0f, B = 0.0f, A = 1.0f;

    // 1. Crtanje narukvice (traka)
    // Gornji deo narukvice (široki pravougaonik iznad ekrana)
    drawQuad(
        0.0f, 0.7f, // Pozicija (centar)
        0.2f, 0.3f, // Polu-širina, Polu-visina
        R, G, B, A
    );
    // Donji deo narukvice
    drawQuad(
        0.0f, -0.7f,
        0.2f, 0.3f,
        R, G, B, A
    );

    // 2. Crtanje Osmerougla / Ekrana sata (zaobljena ivica)
    // Koristimo kombinaciju dva pravougaonika da simuliramo osmerougao.

    // A. Veliki centralni pravougaonik (Glavni deo ekrana)
    drawQuad(
        0.0f, 0.0f,
        FRAME_SIZE_X, FRAME_SIZE_Y,
        R, G, B, A
    );

    // B. Horizontalne "ručke" (iznad i ispod ekrana sata, crne)
    // Gornja ručka:
    drawQuad(
        0.0f, FRAME_SIZE_Y + 0.05f,
        FRAME_SIZE_X + 0.05f, 0.05f,
        R, G, B, A
    );
    // Donja ručka:
    drawQuad(
        0.0f, -FRAME_SIZE_Y - 0.05f,
        FRAME_SIZE_X + 0.05f, 0.05f,
        R, G, B, A
    );

    // C. Bočni pravougaonici (koji simuliraju stranice osmerougla)
    // Iako slika traži osmerougao, ovo je najjednostavniji način bez složene rotacije/geometrije.
    // Ako želimo baš osmerougao (kao na slici), to zahteva složeniju definiciju verteksa.
    // Za sada se držimo jednostavne crne pozadine koja je centralni deo okvira.

    // 3. Iscrtavanje Bele Površine Ekrana unutar Okvira
    // Ovo briše centralni deo, ostavljajući samo crni okvir.
    // Pozicija: 0.0, 0.0
    // Veličina (malo manja od crnog okvira): X: 0.33, Y: 0.38
    drawQuad(
        0.0f, 0.0f,
        FRAME_SIZE_X - 0.02f, FRAME_SIZE_Y - 0.02f,
        1.0f, 1.0f, 1.0f, 1.0f // Bela boja (Ekran)
    );
}

void drawTimeScreen(GLFWwindow* window) {
    drawWatchFrame();
    // 1. Crtanje sata (trenutno crveni kvadrat kao placeholder za vreme)
    // Centar ekrana (0, 0), veličina 0.2
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Formatiranje vremena
    std::string timeStr =
        (hours < 10 ? "0" : "") + std::to_string(hours) + ":" +
        (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
        (seconds < 10 ? "0" : "") + std::to_string(seconds);

    // --- Pozicioniranje teksta ---
    // Želimo da vreme bude u NDC centru ekrana sata (0, 0), tj. u pikselima (width/2, height/2)

    float scale = 1.0f; // Skala (veličina fonta)
    float fontSize = 48.0f * scale; // Font je učitan na 48px

    // Gruba procena širine teksta (prosečno 0.6 širine fonta * broj karaktera)
    float textWidthEstimate = 0.6f * fontSize * timeStr.length();

    // Pozicija X za centriranje: (Piksel Centar X) - (Polu-širina Teksta)
    float posX = (width / 2.0f) - (textWidthEstimate / 2.0f);

    // Pozicija Y za centriranje: (Piksel Centar Y) - (Polu-visina Teksta)
    float posY = (height / 2.0f) + (fontSize / 2.0f) - 5; // Mala korekcija za vertikalno poravnanje

    // Crtanje crnog vremena
    renderText(timeStr, posX, posY, scale, 0.0f, 0.0f, 0.0f, height);

    // 3. Crtanje strelice (plavi kvadrat/pravougaonik)
    // Pomerite ga skroz desno, unutar belog ekrana.
    // X pozicija: npr. 0.2
    const float ARROW_POS_X = FRAME_SIZE_X - 0.10f; // Postavljanje blizu desne ivice
    const float ARROW_POS_Y = 0.0f;
    const float ARROW_WIDTH = 0.05f;
    const float ARROW_HEIGHT = 0.05f;

    // Plava boja za strelicu
    drawQuad(
        ARROW_POS_X, ARROW_POS_Y,
        ARROW_WIDTH, ARROW_HEIGHT,
        0.0f, 0.0f, 1.0f, 1.0f
    );

    // NAPOMENA: Vrednosti ARROW_X_MIN/MAX iz mouse_button_callback su definisane
    // kao AABB (Axis-Aligned Bounding Box) za celu oblast, ne za centar kvadrata.
    // Proverite da li se koordinate poklapaju sa ARROW_POS i ARROW_WIDTH/HEIGHT!
    // Ako koristite drawQuad, njegov X/Y je CENTAR kvadrata.
    // Ako se koristi širina 0.1, onda je NDC X opseg [0.4-0.1, 0.4+0.1] = [0.3, 0.5].
    // Ako se koristi visina 0.05, onda je NDC Y opseg [0.0-0.05, 0.0+0.05] = [-0.05, 0.05].
    // Trebalo bi da ažuriramo logiku provere klika da bude:
    /* const float ARROW_X_MIN = ARROW_POS_X - ARROW_WIDTH;
    const float ARROW_X_MAX = ARROW_POS_X + ARROW_WIDTH;
    const float ARROW_Y_MIN = ARROW_POS_Y - ARROW_HEIGHT;
    const float ARROW_Y_MAX = ARROW_POS_Y + ARROW_HEIGHT;
    */
    // Ali za sada ostavimo jednostavne fiksne brojeve.
}

// Placeholder za ostale ekrane
void drawHeartRateScreen(GLFWwindow* window) {
    drawWatchFrame();
    // std::cout << "Screen: HEART_RATE" << std::endl;
}

void drawBatteryScreen(GLFWwindow* window) {
    drawWatchFrame();
    // std::cout << "Screen: BATTERY" << std::endl;
}

void initQuad() {
    // Koordinate verteksa:
    // Trik 1: (-1, 1) - Trik 2: (1, 1)
    //         | \             / |
    // Trik 3: (-1,-1) - Trik 4: (1,-1)
    float vertices[] = {
        // pozicija (x, y)
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f,  1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Konfiguracija atributa verteksa (location 0 u VAO)
    // Svaki verteks ima 2 float vrednosti (x i y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Pretvaranje koordinata kursora (pikseli) u NDC koordinate (-1.0 do 1.0)
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

// Funkcija za obradu klika
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float ndcX, ndcY;
        getCursorNDC(window, xpos, ypos, ndcX, ndcY);

        // Debug ispis za proveru pozicije
        // std::cout << "Klik na: NDC(" << ndcX << ", " << ndcY << ")" << std::endl;

        // Provera sudara i prebacivanje ekrana
        if (currentScreen == TIME_SCREEN) {
            // DEFINE BOUNDS FOR RIGHT ARROW
            // Strelica za promenu ekrana na desno (otkucaji srca)
            // Pošto je sat na sredini, strelica je desno.
            // Pretpostavimo da je strelica u oblasti: x: [0.3, 0.5], y: [-0.1, 0.1]
            // AŽURIRANE KOORDINATE KLIKA za strelicu
            const float ARROW_POS_X = FRAME_SIZE_X - 0.10f;
            const float ARROW_WIDTH = 0.05f;
            const float ARROW_HEIGHT = 0.05f;

            // Granice: Centar +/- Polu-veličina
            const float ARROW_X_MIN = ARROW_POS_X - ARROW_WIDTH;
            const float ARROW_X_MAX = ARROW_POS_X + ARROW_WIDTH;
            const float ARROW_Y_MIN = 0.0f - ARROW_HEIGHT;
            const float ARROW_Y_MAX = 0.0f + ARROW_HEIGHT;

            if (ndcX >= ARROW_X_MIN && ndcX <= ARROW_X_MAX &&
                ndcY >= ARROW_Y_MIN && ndcY <= ARROW_Y_MAX)
            {
                currentScreen = HEART_RATE_SCREEN;
                std::cout << ">>> Prebacivanje na: HEART_RATE_SCREEN" << std::endl;
            }
        }
        // Logiku za ostale ekrane dodajemo kasnije
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // GLEW init
    if (glewInit() != GLEW_OK)
        return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Omogući podršku za 2D teksture (potrebno za FreeType)
    glEnable(GL_TEXTURE_2D);

    rectShader = createShader("rect.vert", "rect.frag");
    colorShader = createShader("color.vert", "color.frag");

    textShader = createShader("text.vert", "text.frag");

    if (initTextRenderer(mode->width, mode->height) != 0)
        return endProgram("FreeType inicijalizacija nije uspela.");

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
