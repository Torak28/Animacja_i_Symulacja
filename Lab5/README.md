# Laboratorium 5 - Obsługa kamery

**Zadania do wykonania:**

 * [x] Zmiana kamery
 * [x] Sterowanie kamery myszą(z obsługa przybliżania)
 * [ ] Obiekty na scenie poruszają się niezależnie

## Wprowadzenie

Ideą tego laboratorium jest rozbicie macierzy modelu-widoku z której korzystaliśmy do tej pory na dwie osobne macierze(modelu i widoku). Pozwoli nam to na poruszanie kamerą w przestrzeni, wokół obiektu oraz na przybliżanie.

## Zmiana kamery

Z racji rozbicia macierzy widoku-modelu, musimy lekko zmienić kod shadera wierzchołków - wprowadzając nowe wejście typu *uniform*, oraz dodać nowy element iloczynu.

```glsl
#version 330 core \n

layout (location = 1) in vec3 aColor;

in vec4 position;
out vec3 ourColor;

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

void main(void) {
    gl_Position = proj_matrix * view_matrix * model_matrix * position;
    ourColor = aColor;
}
```

Co więcej, z naszego punktu widzenia to co wcześniej było macierzą modelu-widoku, teraz jest macierzą modelu. Tym samym wyliczenie tej macierzy w ramach funkcji `render()` pozostaje bez zmian.

```c
glm::mat4 I = glm::mat4(1.0f);
model_matrix = (
    glm::rotate(I, float(currentTime), glm::vec3(0.0f, 0.0f, -1.0f))
    * glm::rotate(I, float(currentTime), glm::vec3(cos(-1.0f), sin(0.0f), 0.0f))
);
```

Pojawia się za to nowa macierz widoku - złożona z trzech, trójwymiarowych wektorów. Wektory *cameraFront* i *cameraUp* są stałe i niezmienne w czasie trwania programy. Zmienia się tylko wartość *cameraPos* i jest ona wyliczana na podstawie ruchu myszy i scrolla.

```c
view_matrix = glm::lookAt(
        cameraPos,
        cameraFront,
        cameraUp
);
```

## Sterowanie kamery myszą(z obsługa przybliżania)

Do zrealizowania tego punktu jest nam potrzebne sterowanie kamerą w zależności od współrzędnych kursora, oraz zestaw funkcji do jego śledzenia.

Wyliczanie wartości położenia kamery(zmiennej *cameraPos*) odbywa się w następujący sposób z wykorzystaniem biblioteki glm:

```c
glm::vec3 front;
front.x = r_r * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
front.y = r_r * sin(glm::radians(pitch));
front.z = r_r * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
cameraPos = front;
```
Zmienne *yaw* i *pitch* związane są z ruchami myszy, zmienna *r_r* jest związana z obsługa kółka myszy.

Obsługa myszy polega na wyliczeniu różnicy pomiędzy chwilą obecną, a ostatnio sprawdzaną. Definiuje nam to 'moc' przesunięcia, jak i jego kierunek.

```c
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
  if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.2f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
      pitch = 89.0f;
  if (pitch < -89.0f)
      pitch = -89.0f;
}
```

Obsługa kółka myszy jest dużo prostsza jako że sprawdzamy tylko kierunek w jednej płaszczyźnie.

```c
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
  if (r_r >= 1.0f && r_r <= 45.0f)
      r_r -= yoffset;
  if (r_r <= 1.0f)
      r_r = 1.0f;
  if (r_r >= 45.0f)
      r_r = 45.0f;
}
```

Oczywiście, by obsłużyć oba eventy, należało dodać do funkcji `main()`:

```c
glfwSetCursorPosCallback(window, mouse_callback);
glfwSetScrollCallback(window, scroll_callback);
```

## Obiekty na scenie poruszają się niezależnie

Niestety nie byłem w stanie się z tym uporać

![nope](https://media2.giphy.com/media/jUwpNzg9IcyrK/giphy.gif?cid=790b76114d619fc2be45e36fbaf2296cc806b8be32824835&rid=giphy.gif)

## Wyniki:

![gif](https://i.imgur.com/tnntlUJ.gif)