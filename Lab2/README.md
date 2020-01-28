# Laboratorium 2 - Wprowadzenie do programowania Shaderów

**Zadania do wykonania:**

* [x] Shader compilation error - obsługa błędów kompilacji
* [x] Wczytywanie kodu shadera z pliku
* [x] Trójkolorowy trójkąt

## Wprowadzenie

Programy pisane w OpenGlu można sprowadzić do poniższej postaci:

```c
int main(void) {
    
    // rozpoczęcie pracy, w tym kompilacja kodu shaderów
    startup();

    // pętla wykonująca ciągłe rysowanie
    while(True){
        render();
    }

    // zakończenie pracy
    shutdown();
}
```

Dokładnie w ten sam sposób prezentuje się struktura wszystkich programów jakie napisałem w ramach tego kursu.

## Shader compilation error - obsługa błędów kompilacji

Jak juz wspomniałem w poprzednim punkcie, kompilacja shaderów odbywa się w ramach funkcji `startup()`, a dokładnie na jej samym początku w ramach wywołania funkcji `compile_shaders()`

```c
void startup() {
    // kompilajca shaderów
    rendering_program = compile_shaders();
    glCreateVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    // bufor wierzchołków
    unsigned int vertex_buffer_object;
    glGenBuffers(1, &vertex_buffer_object);
    float vertices[] = {
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Przekazanie wierzchołków
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Przekazanie kolorów
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
```

Jak widać poniżej, funkcja `compile_shaders()` definiuje tablicę zmiennych *shaders*, do której wczytuje już skompilowane shadery. W pierwszej kolejności obsługiwany jest shader wierzchołków, dalej shader krawędzi.  

```c
GLuint compile_shaders(void) {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    // Wczytanie z pliku
    string vertexSource = readFile("vertex_shader.vert");
    string fragmentSource = readFile("fragment_shader.vert");

    const GLchar *vertex_shader_source = (const GLchar *)vertexSource.c_str();
    const GLchar *fragment_shader_source = (const GLchar *)fragmentSource.c_str();

    // Tablica shaderów
    vector <GLuint> shaders;

    // Shader wierzchołków
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    shaders.push_back(vertex_shader);

    // Sprawdzenie błędów
    shader_error(shaders);


    // Shader krawędzi
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    shaders.push_back(fragment_shader);

    // Sprawdzenie błędów kompilacji
    shader_error(shaders);

    // Stworzenie programu i zlinkowanie(dołączenie) shaderów
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Sprawdzenie błędów linkowania
    linking_error(program, shaders);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Zwrócenie programu
    return program;
}
```

Za każdym razem gdy tablica *shaders* się powiększy, wywoływana jest na niej funkcja `shader_error()`. Wyciąga ona ostatnio dodany element(najświeższy shader) i sprawdza czy zosatł on skompilowany. Jeśli nie, to na wyjście konsoli wypisuje kod błędu.

```c
void shader_error(vector<GLuint> shaders){
    GLuint shader = shaders.back();
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        cout << string(begin(errorLog), end(errorLog)) << endl;

        for(size_t i = 0; i < shaders.size(); i++) {
            glDeleteShader(shaders[i]);
        }

        return;
    }
}
```

Analogicznie działa funkcja `linking_error()`, z tą różnicą, że sprawdza ona czy shadery zostały poprawnie podpięte do programu.

```c
void linking_error(GLuint program, vector<GLuint> shaders){
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        cout << string(begin(infoLog), end(infoLog)) << endl;

        glDeleteProgram(program);

        for(size_t i = 0; i < shaders.size(); i++) {
            glDeleteShader(shaders[i]);
        }

        return;
    }
}
```

**Przykład błędu:**

![https://imgur.com/kLfb0AP.png](https://imgur.com/kLfb0APl.png)

## Wczytywanie kodu shadera z pliku

Odbywa się w ramach funkcji `compile_shaders()` poprzez wywołanie funkcji `readFile()` na pliku z kodem GLSL. Funkcja ta otwiera plik, zaczytuje go linijka po linijce do zmiennej, zamyka i przekazuje zmienną na wyjście.

```c
string readFile(const char *filePath) {
    string content;
    ifstream fileStream(filePath, ios::in);

    if(!fileStream.is_open()) {
        cerr << "Could not read file " << filePath << ". File does not exist." << endl;
        return "";
    }

    string line = "";
    while(!fileStream.eof()) {
        getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}
```

Sam kod shaderów prezentuje się następująco(opis działania w kolejnej części)

**Shader Wierzchołków:**

```glsl
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

void main(void) {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
}
```

**Shader Fragmentów:**

```glsl
#version 330 core

in vec3 ourColor;
out vec4 color;

void main(void) {
    color = vec4(ourColor, 1.0f);
}
```

## Trójkolorowy trójkąt

Rysowanie trójkąta oraz kolorowanie wierzchołków odbywa się w ramach funkcji `startup()` zaraz po kompilacji shaderów. Idęą tego procesu jest zdefiniowanie algorytmu w jaki shadery mają przetwarzać informację, a następnie przekazanie jej do nich. 

### Definicja Shaderów

Zdefiniowany przezemnie Shader wierzchołków pobiera dane z konkretnego miejsca w VAO, czyli zdefiniowanej wcześniej macierzy o konkretnej strukturze(0 warstawa na współrzędna, 1 na kolor), którą w łatwy spoób można populować danymi. Dalej definiuje zmienna wyjściową *ourColor*, która zostanie przekazana dalej do shadera fragmentów. Zadaniem shadera wierzchołków jest określenie pozycji danego wierzchołka. Shader Fragmentów otrzymuje od shadera wierzchołków informację o kolorze, którą to lekko zmienia(dodaje kolejny wymiar) i wykorzystuje do kolorowania.

### Przekazanie danych

Sam trójką jest zdefinowany w następujący sposób:

```c
float vertices[] = {
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
};
```

Tablica `vertices`, przechowuje inforamjcę o współrzędnych danego wierzchołka(X, Y, Z) i jego kolorze(R, G, B). Poniżej przedstawiam przekazanie informacji.

```c
// Przekazanie wierzchołków
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// Przekazanie kolorów
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
```

Dodatkowo posatnowiłem kolorować w sposób ciągły tło, wykorzystując do tego naturę funkcji `render()`. Definiuję sobie kolor w zależny od czasu działania programu, a następnie wykorzystuje go przy odświeżaniu kolejnych klatek.

```c
void render(double currentTime) {
    const GLfloat color[] = {
        (float) sin(currentTime) * 0.5f + 0.5f,
        (float) cos(currentTime) * 0.5f + 0.5f,
        0.0f,
        1.0f
    };
    glClearBufferfv(GL_COLOR, 0, color);

    glUseProgram(rendering_program);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
```

## Wynik prac:

![gif](https://i.imgur.com/qMBMhv5.gif)