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

GLint model_location;
GLint proj_location;
GLint view_location;

float aspect;
glm::mat4 model_matrix;
glm::mat4 view_matrix;
glm::mat4 proj_matrix;

using namespace std;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos   = glm::vec3(3.0f, 3.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;

float r_r = 10.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;


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

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse)
    {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    if (r_r >= 1.0f && r_r <= 45.0f)
        r_r -= yoffset;
    if (r_r <= 1.0f)
        r_r = 1.0f;
    if (r_r >= 45.0f)
        r_r = 45.0f;
}

static void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    aspect = (float) width / (float) height;
    proj_matrix = glm::perspective(glm::radians(r_r), aspect, 0.1f, 1000.0f);


}

GLuint compile_shaders(void) {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    static const GLchar * vertex_shader_source[] = {
        "#version 330 core \n"

        "layout (location = 1) in vec3 aColor;"

        "in vec4 position;"
        "out vec3 ourColor;"

        "uniform mat4 model_matrix;"
        "uniform mat4 proj_matrix;"
        "uniform mat4 view_matrix;"

        "void main(void) {"
        "    gl_Position = proj_matrix * view_matrix * model_matrix * position;"
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
    glCreateVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    unsigned int vertex_buffer_object;
    glGenBuffers(1, &vertex_buffer_object);
    

    static const GLfloat vertex_positions[] = {
        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,
        -0.25f, -0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
         0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,

         0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,
         0.25f,  0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,

         0.25f, -0.25f, -0.25f,  0.0f, 0.0f, 1.0f,
         0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,
         0.25f,  0.25f, -0.25f,  1.0f, 0.0f, 0.0f,

         0.25f, -0.25f,  0.25f,  1.0f, 0.0f, 0.0f,
         0.25f,  0.25f,  0.25f,  0.0f, 1.0f, 0.0f,
         0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,

         0.25f, -0.25f,  0.25f,  0.0f, 0.0f, 1.0f,
        -0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,
         0.25f,  0.25f,  0.25f,  1.0f, 0.0f, 0.0f,

        -0.25f, -0.25f,  0.25f,  1.0f, 0.0f, 0.0f,
        -0.25f,  0.25f,  0.25f,  0.0f, 1.0f, 0.0f,
         0.25f,  0.25f,  0.25f,  0.0f, 0.0f, 1.0f,

        -0.25f, -0.25f,  0.25f,  0.0f, 0.0f, 1.0f,
        -0.25f, -0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
        -0.25f,  0.25f,  0.25f,  1.0f, 0.0f, 0.0f,

        -0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,
        -0.25f,  0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
        -0.25f,  0.25f,  0.25f,  0.0f, 0.0f, 1.0f,

        -0.25f, -0.25f,  0.25f,  0.0f, 0.0f, 1.0f,
         0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,
         0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,

         0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,
        -0.25f, -0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
        -0.25f, -0.25f,  0.25f,  0.0f, 0.0f, 1.0f,

        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,
         0.25f,  0.25f, -0.25f,  0.0f, 1.0f, 0.0f,
         0.25f,  0.25f,  0.25f,  1.0f, 0.0f, 0.0f,

         0.25f,  0.25f,  0.25f,  1.0f, 0.0f, 0.0f,
        -0.25f,  0.25f,  0.25f,  0.0f, 1.0f, 0.0f,
        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f
    };

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);
}

void shutdown() {
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(rendering_program);
    glDeleteBuffers(1, &buffer);
}

void render(double currentTime) {
    float f = (float) currentTime * (float) M_PI * 0.1f;
    glm::mat4 I = glm::mat4(1.0f);
    model_matrix = (
        glm::rotate(I, float(currentTime), glm::vec3(0.0f, 0.0f, -1.0f))
        * glm::rotate(I, float(currentTime), glm::vec3(cos(-1.0f), sin(0.0f), 0.0f))
    );
    

     const GLfloat color[] = {
        (float) sin(currentTime) * 0.5f + 0.5f,
        (float) cos(currentTime) * 0.5f + 0.5f,
        0.5f,
        1.0f
    };
    const GLfloat xd = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &xd);

    glUseProgram(rendering_program);

    glm::vec3 front;
    front.x = r_r * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = r_r * sin(glm::radians(pitch));
    front.z = r_r * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraPos = front;
    view_matrix = glm::lookAt(
            cameraPos,
            cameraFront,
            cameraUp
    );

    proj_location = glGetUniformLocation(rendering_program, "proj_matrix");
    view_location = glGetUniformLocation(rendering_program, "view_matrix");
    model_location = glGetUniformLocation(rendering_program, "model_matrix"); 
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));

    for (int i = 0;i < 2; ++i) {
        model_matrix = (
            glm::translate(model_matrix, glm::vec3(0.0f, sin(2.0f), -1.0 * i))
        );
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

int main(void) {
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if(! glfwInit())
        return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


    proj_matrix = glm::perspective(glm::radians(r_r), 800/600.0f, 0.1f, 1000.0f);


    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab 5", NULL, NULL);
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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
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