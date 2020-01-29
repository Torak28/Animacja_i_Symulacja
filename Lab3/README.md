# Laboratorium 3 - Ruch bryły 3D

**Zadania do wykonania:**

 * [x] Ruch obiektu,
 * [x] Kolorowanie ścian,
 * [x] Rysowanie przy pomocy Triangle_Strip

## Wprowadzenie

Sama struktura programu, jego idea i budowa zostały niezmienione w stosunku do poprzedniego laboratorium. Na potrzeby zadania w ramach tego laboratorium napisałem dwa osobne programy *main.c* i *main_strip.c*.

## Ruch obiektu

By zdefiniować ruch obiektu musimy posłużyć się przekształceniami macierzowymi. Wykorzystamy do tego macierz modelu-widoku(*mv_matrix*) i macierz projekcji(*proj_matrix*). Zaczniemy od zdefiniowania ich w ramach shadera wierzchołków:

```glsl
#version 330 core \n

layout (location = 1) in vec3 aColor;

in vec4 position;
out vec3 ourColor;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main(void) {
    gl_Position = proj_matrix * mv_matrix * position;
    ourColor = aColor;
}
```

Ich przekazanie odbędzie się poprzez mechanizm *uniform*. Zmienne korzystające z tego mechanizmu są globalne, co oznacza że po zdefiniowaniu mogą być używane przez shadery na każdym etapie pracy. Jako że chcemy wykorzystać je do wprawienia naszego obiektu w ruch, wywołamy je podobnie w jak poprzednim laboratorium zmianę koloru tła, w ramach funkcji `render()` i jako jeden z parametrów przyjmiemy czas.

Do wyliczenia macierzy modelu-widoku skorzystamy z funkcji z biblioteki glm - `rotate()` i `translate()`. Pierwsza z nich odpowiedzialna jest za obrót wokół konkretnej osi, druga odpowiedzialna jest za za wyliczenie translacji(przesunięcia).

```c
void render(double currentTime) {
  float f = (float) currentTime * (float) M_PI * 0.1f;

  // definicja macierzy obrotu, wyrażonego w stopniach(inkrementacja o kąt w jednostce czasu)
  glm::mat4 I = glm::mat4(1.0f);
  mv_matrix = (
      glm::rotate(I, float(currentTime), glm::vec3(0.0f, 0.0f, -1.0f)) * glm::translate(I, glm::vec3((float) cos(0.0f), (float) sin(0.0f), -5.0f)) * glm::rotate(I, float(currentTime), glm::vec3(-1.0f, 1.0f, 0.0f))
  );

  // kolor tła wyznaczony na poprzednich laboratoriach
  const GLfloat color[] = {
      (float) sin(currentTime) * 0.5f + 0.5f,
      (float) cos(currentTime) * 0.5f + 0.5f,
      0.0f,
      1.0f
  };
  glClearBufferfv(GL_COLOR, 0, color);
  glUseProgram(rendering_program);

  // pobanie macierzy i przekazanie je do zmiennych typu uniform
  mv_location = glGetUniformLocation(rendering_program, "mv_matrix");
  proj_location = glGetUniformLocation(rendering_program, "proj_matrix");
  glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

  // narysowanie naszego sześcianu
  glDrawArrays(GL_TRIANGLES, 0, 36);
}
```

Tym samym jak widać w kodzie shadera, umiejscowienie naszego obiektu w scenie jest wyznaczana poprzez iloczyn pozycji(początkowa pozycja wierzchołków jak i ich zależności względem siebie) i macierzy projekcji i modelu-widoku.

## Kolorowanie Ścian

W przeciwieństwie do poprzedniego laboratorium, tym razem mamy do czynienia z sześcianem. Jego ściany pokolorowałem w taki sposób, by przypominały kostkę Rubika. Sama jednak procedura przekazania informacji jest taka sama, jak w poprzednim Laboratorium.

## Rysowanie przy pomocy Triangle_Strip

Tym razem zmieniamy sposób rysowania sześcianu. Do tej pory korzystaliśmy z konkretnych współrzędnych, tym samym chcąc zdefiniować sześcian, musieliśmy rysować po dwa trójkąty na każdą ścianę, co daje nam 12 trójkątów na całą bryłę. 12 trójkątów, z czego każdy składał się z trzech punktów, co daje nam, 36 punktów w sumie. Chcielibyśmy tą ilość zmniejszyć i do tego celu skorzystamy z prymitywu Triangle_Strip.

Wymaga on innego zdefiniowania wierzchołków - podania ich w konkretnej kolejności. Pozwala to na niedefiniowanie krawędzi wewnątrz(połączą się one same), oraz nie podawiania tych wierzchołków po kilka razy. Dzięki czemu oszczędzamy na pamięci. Poniżej przedstawiam rysunek poglądowy dla sześcianu:

![pic](https://www.paridebroggi.com/assets/media/images/2015-06-16-triangle-strip.jpg)

Tym samym jak widać poniżej definiujemy tylko 8 wierzchołków w ramach zmiennej *vertex_positions*(a dalej bufora - *buffer*), redukując wielkość zmiennej o ~60% (poprzednio znajdowało się tutaj 36 elementów). Dalej pojawiają się dalsze bufory, przetrzymujące m.in. indeksy wierzchołków(*buffer3* - do wyświetlania w ramach funkcji `glDrawElements()`) oraz bufor *buffer2* - przetrzymujący kolory konkretnych wierzchołków.

```c
void startup() {
    rendering_program = compile_shaders();
    glCreateVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    unsigned int vertex_buffer_object;
    glGenBuffers(1, &vertex_buffer_object);

    // Wierzchołki
    static const GLfloat vertex_positions[] = {
        0.25f,  0.25f,  0.25f,
       -0.25f,  0.25f,  0.25f,

        0.25f,  0.25f, -0.25f,
       -0.25f,  0.25f, -0.25f,

        0.25f, -0.25f,  0.25f,
       -0.25f, -0.25f,  0.25f,

       -0.25f, -0.25f, -0.25f,
        0.25f, -0.25f, -0.25f
    };

    // Indeksy wierzchołków
    static const GLuint elements[] = {
        3, 2, 6, 7, 4, 2, 0,
        3, 1, 6, 5, 4, 1, 0
    };

    // Kolory
    static const GLfloat vertex_col[] = {
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        1.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f,
        0.0f, 0.5f, 1.0f
    };

    //pozycje
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), vertex_positions, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //elementy
    glGenBuffers(1, &buffer3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer3);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 14 * sizeof(GLuint), elements, GL_STATIC_DRAW);

    //kolory
    glGenBuffers(2, &buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_col), vertex_col, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
}
```

Samo wywoałanie funkcji rysującej w ramach funkcji `render()` wygląda następująco:

```c
glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, 0);
```

## Wyniki prac:

**Zwykła kostka:**

![gif](https://i.imgur.com/Lq7i70h.gif)

**Triangle_Strip:**

![gif](https://i.imgur.com/Blutahr.gif)