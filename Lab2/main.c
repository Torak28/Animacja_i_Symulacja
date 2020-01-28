#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

using namespace std;

GLuint rendering_program;
GLuint vertex_array_object;

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

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

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

void shutdown() {
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(rendering_program);
}

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

int main(void) {
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if(! glfwInit())
        return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(800, 600, "Lab2", NULL, NULL);
    if(! window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if(gl3wInit()) {
        fprintf(stderr, "failed to initialize OpenGL\n");
        return -1;
    }
    if(! gl3wIsSupported(3, 3)) {
        fprintf(stderr, "OpenGL 3.3 not supported\n");
        return -1;
    }
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION));

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, resize_callback);
    glfwSwapInterval(1);

    startup();

    while(!glfwWindowShouldClose(window)){
        render(glfwGetTime());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}