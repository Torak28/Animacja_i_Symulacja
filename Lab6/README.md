# Laboratorium 6 - rysowanie indeksowane, metody numeryczne

**Zadania do wykonania:**

 * [x] Rysowanie sześcianu przy pomocy glDrawElements
 * [x] Ruch kuli armatniej

## Wprowadzenie

Celem zajęć jest zmiana funkcji rysującej z `glDrawArrays()` na `glDrawElements()` oraz wykorzystania metod numerycznych do analizy ruchu pocisku armatniego z uwzględnianiem oporu powietrza. 

## Rysowanie sześcianu przy pomocy glDrawElements

Do zmiany funkcji rysującej, musimy zmienić strukturę wierzchołków. Tym razem pełnią one role pomocniczą, jako że posiadamy dedykowaną tabelę *vertex_ind*, która posiada ich odpowiednie indeksy. Tym samym tabela wierzchołków *vertex_all_pos* wypełniona jest po prostu wszystkimi kombinacjami koordynatów.

```c
// wierzchołki
static const GLfloat vertex_all_pos[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,
};

// indeksy
static const GLint vertex_ind[] = {
    0, 1, 2,
    1, 3, 2,
    7, 5, 1,
    7, 1, 3,
    6, 4, 5,
    6, 5, 7,
    6, 4, 0,
    6, 0, 2,
    6, 7, 3,
    6, 3, 2,
    4, 5, 1,
    4, 1, 0
};

// kolory
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
```

W następnej kolejności następuje przypisanie do odpowiednich buforów:

```c
//pozycje
glGenBuffers(1, &buffer);
glBindBuffer(GL_ARRAY_BUFFER, buffer);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_all_pos), vertex_all_pos, GL_STATIC_DRAW);

glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

//kolory
glGenBuffers(2, &buffer2);
glBindBuffer(GL_ARRAY_BUFFER, buffer2);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_col), vertex_col, GL_STATIC_DRAW);

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(1);

//indexy
glGenBuffers(3, &buffer3);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer3);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_ind), vertex_ind, GL_STATIC_DRAW);
```

i wywołanie nowej funkcji rysującej:

```c
glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
```

![gif](https://i.imgur.com/H3MQTha.gif)

## Ruch kuli armatniej

Naszym zadaniem było wyznaczenie punktów w których znajdzie się obiekt, posługując się zadanymi wzorami i wartościami początkowymi.

![https://imgur.com/co0d8JF.png](https://imgur.com/co0d8JF.png)

Jak widać występuje w kodzie zmienna *k*, będąca wartością oporu powietrza. W oparciu o różne jej wartości sporządziłem poniższy wykres. Jak widać im większy opór, tym mniejsza droga jaką nasz pocisk pokona. Co więcej im większa wartość *k* tym przebieg funkcji staje się bardziej ostry/szpiczasty.

![pic](https://raw.githubusercontent.com/Torak28/Animacja_i_Symulacja/master/Lab6/k10.png)

Samo wyliczanie wartości polegało na przepisaniu wzorów matematycznych i wrzucenia ich w pętlę kręcącą się dopóki kula nie zderzy się z ziemią.

```py
def count_and_save(var_k):
    # Wartośći początkowe
    x = 0
    y = 0
    k = var_k
    delta_t = 0.01
    g = 9.81
    V_pocz = 10
    alfa = math.pi / 4
    V_x = V_pocz * math.cos(alfa)
    V_y = V_pocz *math.sin(alfa)

    # Funkcja do wyliczenia położeń
    def foo(V_x, V_y, x, y):
        V_x_new = V_x - k * V_x * delta_t
        V_y_new = V_y - g * delta_t - k * V_y * delta_t
        x_new = x + V_x * delta_t - 1/2 * pow(k, 2) * V_x * pow(delta_t, 2)
        y_new = y + V_y * delta_t - 1/2 * g * pow(delta_t, 2) - 1/2 * pow(k, 2) * V_y * pow(delta_t, 2)
        
        return V_x_new, V_y_new, x_new, y_new 

    x_tab = []
    y_tab = []

    # Pętla wykonująca sie do momentu w którym kula spadanie na ziemie
    while y >= 0:
        new_V_x, new_V_y, new_x, new_y = foo(V_x, V_y, x, y)
        V_x = new_V_x
        V_y = new_V_y
        x = new_x
        y = new_y
        x_tab.append(x)
        y_tab.append(y)

    # Moduł tworzący wykres
    title = 'k = ' + str(k)
    plt.plot(x_tab, y_tab, label=title)
    plt.title('Wpływ wartośći k')
    plt.legend()
    filetitle = 'k' + str(k) + '.png'
    plt.savefig(filetitle)
```