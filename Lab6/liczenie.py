
import math
import matplotlib.pyplot as plt

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

count_and_save(0.1)
count_and_save(1)
count_and_save(2)
count_and_save(5)
count_and_save(10)
