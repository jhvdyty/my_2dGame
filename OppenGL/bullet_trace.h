#ifndef BULLET_TRACE_H
#define BULLET_TRACE_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "stb_image.h"
#include <iostream>

class BulletTrace {
private:
    glm::vec2 position;
    float size;
    float lifeTime;
    float maxLifeTime;
    unsigned int VAO, VBO;
    unsigned int texture;
    Shader shader;
    int screenWidth, screenHeight;

public:
    BulletTrace(float x, float y, float size, float maxLifeTime, const char* texturePath, Shader& shader, int width, int height)
        : position(x, y), size(size), lifeTime(0.0f), maxLifeTime(maxLifeTime), shader(shader), screenWidth(width), screenHeight(height) {
        setupMesh();
        texture = loadTexture(texturePath);
    }

    void setupMesh() {
        float vertices[] = {
            // Позиции      // Текстурные координаты
            -size,  size,  0.0f, 0.0f,
             size,  size,  1.0f, 0.0f,
             size, -size,  1.0f, 1.0f,
            -size, -size,  0.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    unsigned int loadTexture(const char* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;  // Возвращаем ID текстуры
    }

    void update(float deltaTime) {
        lifeTime += deltaTime;
    }

    bool isAlive() const {
        return lifeTime < maxLifeTime;
    }


    void draw() {
        if (!isAlive()) return;
        shader.Use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        float alpha = 1.0f - (lifeTime / maxLifeTime);
        glUniform1f(glGetUniformLocation(shader.Program, "alpha"), alpha);

        float aspectRatio = static_cast<float>(screenWidth) / screenHeight;
        glUniform1f(glGetUniformLocation(shader.Program, "aspectRatio"), aspectRatio);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0); 
    }

    ~BulletTrace() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

#endif
