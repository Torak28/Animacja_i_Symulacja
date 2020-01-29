# Laboratorium 4 - Instancje wielu obiektów

**Zadania do wykonania:**

 * [x] Seria sześcianów,
 * [x] Model źdźbła trawy,
 * [x] Pole trawy,
 * [ ] Losowe zmiany poszczególnych źdźbeł w ramach pola trawy.

## Wprowadzenie

Na potrzeby laboratorium zostały stworzone dwa programy. Pierwszy `main.c` zawierający prezentację serii sześcianów i drugi `main_grass.c` zawierający rozwiązanie pola trawy.

## Seria sześcianow

Wygenerowanie serii sześcianów sprowadziło się tylko do lekkiej zmiany pod koniec funkcji `render()`. Stworzyłem tam pętle *for* w środku której przeliczam i przesyłam macierz widoku-modelu wykorzystując zmienna *i*(zmienna sterująca funkcji for). Działanie dostępne na gifie widocznym pod koniec tego sprawozdania.

```c
for (int i = 0; i < 6; ++i) {
    mv_matrix = (
        glm::translate(mv_matrix, glm::vec3(0.0f, sin(2.0f), -1.0*i))
    );
    glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    }
```

## Model źdźbła trawy

Pierw zbudowałem samo pojedyncze źdźbło, korzystając z 5 trójkątów, kolorując każdy na różny kolor, tak żeby łatwo je identyfikować.

```c
static const GLfloat vertex_positions[] = {
    0.00f,  0.05f,  1.0f, 0.0f, 0.0f,
    0.05f, -0.05f,  1.0f, 0.0f, 0.0f,
    0.00f, -0.05f,  1.0f, 0.0f, 0.0f,

    0.00f,  0.05f,  1.0f, 0.2f, 0.0f,
    0.05f, -0.05f,  1.0f, 0.2f, 0.0f,
    0.05f,  0.05f,  1.0f, 0.2f, 0.0f,

    0.00f,  0.15f,  0.0f, 1.0f, 0.0f,
    0.05f,  0.05f,  0.0f, 1.0f, 0.0f,
    0.00f,  0.05f,  0.0f, 1.0f, 0.0f,

    0.00f,  0.15f,  0.0f, 1.0f, 0.4f,
    0.05f,  0.05f,  0.0f, 1.0f, 0.4f,
    0.05f,  0.15f,  0.0f, 1.0f, 0.4f,

    0.025f,  0.25f,  0.0f, 0.0f, 1.0f,
    0.05f,  0.15f,  0.0f, 0.0f, 1.0f,
    0.00f,  0.15f,  0.0f, 0.0f, 1.0f
};
```

Samą idę przesyłania wierzchołków i kolorów do shadera opisałem juz przy wcześniejszych sprawozdaniach, więc na ten moment ten opis pominę. Poniżej przybliżenie na parę źdźbeł(Nie mam niestety renderu pojedynczego, pracowałem nad tym podpunktem i następnym jednocześnie).

![https://imgur.com/33To2Cg.png](https://imgur.com/33To2Cgl.png)

## Pole trawy

W celu zbudowania pola trawy stworzyłem przed deklaracja wierzchołków zmienną tabelę *translations* o wielkości 10000, przechowującą translację dla każdego elementu.

```c
glm::vec2 translations[10000];
int index = 0;
float offset = 0.1f;
for (int y = -50; y < 50; y += 2)
{
    for (int x = -100; x < 100; x += 1)
    {
        glm::vec2 translation;
        translation.x = (float)x / 10.0f + offset;
        translation.y = (float)y / 5.0f + offset;
        translations[index++] = translation;
    }
}
```

Parametry pętli for i przeliczenia poszczególnych X i Y zostały dobrane na oko, tak żeby całość wyglądała dość wiarygodnie. Tabela ta jest dalej przesyłana do shadera z wykorzystaniem bufora.

Tym samym w funkcji render występuje następująca funkcja rysująca:

```c
glDrawArraysInstanced(GL_TRIANGLES, 0, 15, 10000);
```

Dodam może jeszcze tylko dwie rzeczy. Wykorzystałem macierz modelu-widoku do skalowania całego tworu.

```c
glm::mat4 I = glm::mat4(1.0f);
mv_matrix = (
    glm::scale(I, glm::vec3(0.15f, 0.2f, 1.0f))
);
```

Co sprawiło że kod shadera wierzchołków wygląda następująco:

```glsl
#version 330 core \n

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;

uniform mat4 mv_matrix;
out vec3 ourColor;

void main(void) {
    gl_Position = vec4(aPos + aOffset, 0.0, 1.0) * mv_matrix;
    ourColor = aColor;
}
```

Jak widać dalej używam metod z poprzednich laboratoriów do przesyłania informacji.

## Losowe zmiany poszczególnych źdźbeł w ramach pola trawy

Tutaj niestety ale napotkałem pewnie problemy.

![sad](https://media.giphy.com/media/l3q2Y4Czu8vJ5FRx6/giphy.gif)

## Wynik prac:

![gif](https://i.imgur.com/n7IDRYf.gif)

![https://imgur.com/8Bt010Y.png](https://imgur.com/8Bt010Yl.png)