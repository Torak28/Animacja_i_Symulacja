#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


GLuint rendering_program;
GLuint vertex_array_object;
GLuint buffer;
GLuint buffer2;
GLuint buffer3;

GLint rand1_location;
GLint rand2_location;

float aspect;
glm::mat4 mv_matrix;
glm::mat4 proj_matrix;

float rand1;
float rand2;

using namespace std;

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

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    aspect = (float) width / (float) height;
    proj_matrix = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
}

float RandomFloat(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

GLuint compile_shaders(void) {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    static const GLchar * vertex_shader_source[] = {
        "#version 330 core \n"

        "layout (location = 0) in vec2 aPos;"
        "layout (location = 1) in vec3 aColor;"
        "layout (location = 2) in vec2 aOffset;"

        "uniform float rand2;"
        "uniform float rand1;"
        "out vec3 ourColor;"

        "void main(void) {"
        "    vec2 pos = aPos * (gl_InstanceID / rand2) + rand1;"
        "    gl_Position = vec4(pos + aOffset, 0.0, 1.0);"
        "    ourColor = aColor;"
        "}"
    };

    static const GLchar * fragment_shader_source[] = {
        "#version 330 core \n"

        "in vec3 ourColor;"
        "out vec4 color;"

        "void main(void) {"
        "    color = vec4(ourColor, 1.0);"
        "}"
    };

    vector <GLuint> shaders;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
    glCompileShader(vertex_shader);    
    shaders.push_back(vertex_shader);

    shader_error(shaders);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    shaders.push_back(fragment_shader);

    shader_error(shaders);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    linking_error(program, shaders);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void startup() {
    rendering_program = compile_shaders();

    glm::vec2 translations[100];
    int index = 0;
    float offset = 0.1f;
    for (int y = -10; y < 10; y += 2)
    {
        for (int x = -5; x < 5; x += 1)
        {
            glm::vec2 translation;
            translation.x = (float)x / 10.0f + offset;
            translation.y = (float)y / 10.0f + offset;
            translations[index++] = translation;
        }
    }

    unsigned int vertex_buffer_object;
    glGenBuffers(1, &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    static const GLfloat vertex_positions[] = {
        // positions     // colors
       -0.03f,  0.03f,  1.0f, 0.0f, 0.0f,
        0.03f, -0.03f,  1.0f, 0.0f, 0.0f,
       -0.03f, -0.03f,  1.0f, 0.0f, 0.0f,

       -0.03f,  0.03f,  1.0f, 0.2f, 0.0f,
        0.03f, -0.03f,  1.0f, 0.2f, 0.0f,
        0.03f,  0.03f,  1.0f, 0.2f, 0.0f,

       -0.03f,  0.09f,  0.0f, 1.0f, 0.0f,
        0.03f,  0.03f,  0.0f, 1.0f, 0.0f,
       -0.03f,  0.03f,  0.0f, 1.0f, 0.0f,

       -0.03f,  0.09f,  0.0f, 1.0f, 0.4f,
        0.03f,  0.03f,  0.0f, 1.0f, 0.4f,
        0.03f,  0.09f,  0.0f, 1.0f, 0.4f,

       -0.00f,  0.15f,  0.0f, 0.0f, 1.0f,
        0.03f,  0.09f,  0.0f, 0.0f, 1.0f,
       -0.03f,  0.09f,  0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &buffer);
    glBindVertexArray(buffer);

    glGenBuffers(1, &buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);

    glEnable(GL_DEPTH_TEST);
}

void shutdown() {
    glDeleteVertexArrays(1, &buffer);
    glDeleteBuffers(1, &buffer2);
    glDeleteProgram(rendering_program);
}

void render(double currentTime) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(rendering_program);

    rand1_location = glGetUniformLocation(rendering_program, "rand1");
    rand2_location = glGetUniformLocation(rendering_program, "rand2");
    glUniform1f(rand1_location, rand1);
    glUniform1f(rand2_location, rand2);

    glBindVertexArray(buffer);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 15, 100);
    glBindVertexArray(0);
}

int main(void) {
    GLFWwindow* window;

    rand1 = RandomFloat(0.00f, 0.05f);
    rand2 = RandomFloat(95.0f, 97.0f);

    glfwSetErrorCallback(error_callback);

    if(! glfwInit())
        return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
    proj_matrix = glm::perspective(glm::radians(50.0f), 800/600.0f, 0.1f, 1000.0f);

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
