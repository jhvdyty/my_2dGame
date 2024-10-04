#pragma once
#ifndef COLLIDE_H
#define COLLIDE_H

#include <glad/glad.h> 

#include <string> 
#include <fstream> 
#include <sstream> 
#include <iostream> 

#include "shader.h"
#include "stb_image.h"

#include <GLFW/glfw3.h> 

class Collide {
private:
    float x, y;
    float speed;
    float width, height;
    unsigned int VAO, VBO, EBO;
    Shader shader;

public:
    float getX() { return x; }
    float getY() { return y; }
    float getWidth() { return width; }
    float getHeight() { return height; }

    Collide(float startX, float startY, float characterWidth, float characterHeight, float moveSpeed, const char* vertexPath, const char* fragmentPath)
        : x(startX), y(startY), width(characterWidth), height(characterHeight), speed(moveSpeed), shader(vertexPath, fragmentPath) {
        setupMesh();
    }

    void setupMesh() {
        float vertices[] = {
            // Positions                   // Colors           // Texture Coords
             width / 2,  height / 2, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
             width / 2, -height / 2, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
            -width / 2, -height / 2, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
            -width / 2,  height / 2, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left 
        };

        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void draw() {
        shader.Use();

        glUniform1f(glGetUniformLocation(shader.Program, "x_mov_full"), x);
        glUniform1f(glGetUniformLocation(shader.Program, "y_mov_full"), y);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~Collide() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

};




#endif