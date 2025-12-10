#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Util.h"

#define NUM_SLICES 40

// Main fajl funkcija sa osnovnim komponentama OpenGL programa

// Projekat je dozvoljeno pisati počevši od ovog kostura
// Toplo se preporučuje razdvajanje koda po fajlovima (i eventualno potfolderima) !!!
// Srećan rad!

unsigned signatureTex;

unsigned int VBOsignature;

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void formVAOTexture(float* vertices, size_t size, unsigned int& vao) {
    // formiranje VAO-ova je izdvojeno u posebnu funkciju radi čitljivijeg koda u main funkciji

    // Podsetnik za atribute:
    /*
        Jedan VAO se vezuje za jedan deo celokupnog skupa verteksa na sceni.
        Na primer, dobra praksa je da se jedan VAO vezuje za jedan VBO koji se vezuje za jedan objekat, odnosno niz temena koja opisuju objekat.

        VAO je pomoćna struktura koja opisuje kako se podaci u nizu objekta interpretiraju.
        U render-petlji, za crtanje određenog objekta, naredbom glBindVertexArray(nekiVAO) se određuje koji se objekat crta.

        Potrebno je definisati koje atribute svako teme u nizu sadrži, npr. pozicija na lokaciji 0 i boja na lokaciji 1.

        Ova konfiguracija je specifična za naš primer na vežbama i može se menjati za različite potrebe u projektu.


        Atribut se opisuje metodom glVertexAttribPointer(). Argumenti su redom:
            index - identifikacioni kod atributa, u verteks šejderu je povezan preko svojstva location (location = 0 u šejderu -> indeks tog atributa treba staviti isto 0 u ovoj naredbi)
            size - broj vrednosti unutar atributa (npr. za poziciju su x i y, odnosno 2 vrednosti; za boju r, g i b, odnosno 3 vrednosti)
            type - tip vrednosti
            normalized - da li je potrebno mapirati na odgovarajući opseg (mi poziciju već inicijalizujemo na opseg (-1, 1), a boju (0, 1), tako da nam nije potrebno)
            stride - koliko elemenata u nizu treba preskočiti da bi se došlo od datog atributa u jednom verteksu do istog tog atributa u sledećem verteksu
            pointer - koliko elemenata u nizu treba preskočiti od početka niza da bi se došlo do prvog pojavljivanja datog atributa
    */
    // Četvorougao

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &VBOsignature);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, VBOsignature);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Atribut 0 (pozicija):
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atribut 1 (tekstura):
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void formVAOPositionOnly(float* vertices, size_t size, unsigned int& vao) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &VBOsignature);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, VBOsignature);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Atribut 0 (pozicija):
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawSignature(unsigned int shader, unsigned int vao) {
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signatureTex);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

struct Station {
    float x, y;
    int number;
};

Station stations[10];

void initializeStations() {
    stations[0].x = -0.4f; stations[0].y = 0.6f; stations[0].number = 0;
    stations[1].x = 0.15f; stations[1].y = 0.55f; stations[1].number = 1;
    stations[2].x = 0.5f; stations[2].y = 0.65f; stations[2].number = 2;
    stations[3].x = 0.55f; stations[3].y = 0.3f; stations[3].number = 3;
    stations[4].x = 0.65f; stations[4].y = -0.35f; stations[4].number = 4;
    stations[5].x = 0.1f; stations[5].y = -0.5f; stations[5].number = 5;
    stations[6].x = -0.15f; stations[6].y = -0.65f; stations[6].number = 6;
    stations[7].x = -0.4f; stations[7].y = -0.1f; stations[7].number = 7;
    stations[8].x = -0.75f; stations[8].y = 0.15f; stations[8].number = 8;
    stations[9].x = -0.45f; stations[9].y = 0.25f; stations[9].number = 9;
}

unsigned stationNumberTextures[10];

void drawStations(unsigned int shader, unsigned int vao) {
    glUseProgram(shader);

    GLint loc = glGetUniformLocation(shader, "uOffset");

    for (int i = 0; i < 10; i++) {
        glUniform2f(loc, stations[i].x, stations[i].y);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_SLICES + 2);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Bus Route Tracker", monitor, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    glfwSetCursor(window, loadImageToCursor("../Resources/bus_stop.png"));

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    preprocessTexture(signatureTex, "../Resources/signature.png");

    unsigned int signatureShader = createShader("../Shaders/signature.vert", "../Shaders/signature.frag");
    glUseProgram(signatureShader);
    glUniform1i(glGetUniformLocation(signatureShader, "signatureTex"), 0);

    unsigned int stationShader = createShader("../Shaders/station.vert", "../Shaders/station.frag");
    glUseProgram(stationShader);

    float verticesSignature[] = {
        0.5f, -0.7f, 0.0f, 1.0f, // gornje levo teme
        0.5f, -1.0f, 0.0f, 0.0f, // donje levo teme
        1.0f, -1.0f, 1.0f, 0.0f, // donje desno teme
        1.0f, -0.7f, 1.0f, 1.0f, // gornje desno teme
   };

    unsigned int VAOsignature;
    formVAOTexture(verticesSignature, sizeof(verticesSignature), VAOsignature);

    initializeStations();
    float verticesStation[(NUM_SLICES + 2) * 2];

    float xc = 0.0f, yc = 0.0f, r = 0.12f;
    float aspect = (float)mode->width / (float)mode->height;

    verticesStation[0] = xc;
    verticesStation[1] = yc;

    for (int i = 1; i < NUM_SLICES + 2; ++i) {
        float angle = i * 2 * M_PI / NUM_SLICES;
        float x = cos(angle) * r / aspect + xc; // divide x by aspect
        float y = sin(angle) * r + yc;
        verticesStation[i * 2 + 0] = x;
        verticesStation[i * 2 + 1] = y;
    }

    unsigned int VAOstations;
    formVAOPositionOnly(verticesStation, sizeof(verticesStation), VAOstations);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        double initFrameTime = glfwGetTime();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT);

        drawSignature(signatureShader, VAOsignature);
        drawStations(stationShader, VAOstations);

        glfwSwapBuffers(window);
        glfwPollEvents();

        while (glfwGetTime() - initFrameTime < 1 / 75.0) {}
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}