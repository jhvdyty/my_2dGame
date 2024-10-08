#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"

class Crosshair {
private:
    unsigned int VAO, VBO;
    Shader shader;
    float size;

public:
    Crosshair(float crosshairSize = 0.03f)
        : shader("vertex_crosshair.glsl", "fragment_crosshair.glsl"), size(crosshairSize) {
        float vertices[] = {
            // Горизонтальная линия
            -size,  0.0f, 0.0f,
             size,  0.0f, 0.0f,
             // Вертикальная линия
              0.0f, -size - 0.03f, 0.0f,
              0.0f,  size + 0.03f, 0.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void draw(GLFWwindow* window) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Преобразование координат экрана в координаты OpenGL
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float normalizedX = (2.0f * xpos) / width - 1.0f;
        float normalizedY = 1.0f - (2.0f * ypos) / height;

        shader.Use();
        glUniform2f(glGetUniformLocation(shader.Program, "crosshairPosition"), normalizedX, normalizedY);
        glUniform3f(glGetUniformLocation(shader.Program, "crosshairColor"), 1.0f, 0.0f, 0.0f); // Красный цвет

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 4);
    }

    ~Crosshair() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

#endif