#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include <glad/glad.h> 

#include <vector>
#include <string> 
#include <fstream> 
#include <sstream> 
#include <iostream> 
#include <random>

#include "shader.h"
#include "stb_image.h"
#include "collide.h"
#include "character.h"

#include <GLFW/glfw3.h>

class ParticleEmitter {
private:

    glm::mat4 projection;

    float particleSpeed;
    float particleDamage;
    float particleLifetime;

    unsigned int texture1;

    float width, height;
    unsigned int VAO, VBO, EBO;

    float quadLeft, quadRight, quadTop, quadBottom;

    float characterWidth = 0.25f;  // Ширина персонажа в игровом мире
    float characterHeight = 0.25f; // Высота персонажа в игровом мире

    int hp;
    bool isAlive;
    
    float attackCooldown;
    float timeSinceLastAttack;
    int damage;

    bool isfallsdown;

    Shader particleShader;

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

        return textureID;
    }

public:
    float x, y; 
    float startX, startY;
    Character* character; 

    ParticleEmitter(float startX, float startY, float characterWidth, float characterHeight, float moveSpeed,
        const char* vertexPath, const char* fragmentPath,
        const char* texturePath,
        float screenWidth, float screenHeight, bool isfallsdown)
        : x(startX), y(startY), width(characterWidth), height(characterHeight), particleSpeed(moveSpeed),
        particleShader(vertexPath, fragmentPath),
        quadLeft(-width / 2), quadRight(width / 2), quadTop(height / 2), quadBottom(-height / 2),
        hp(5), isAlive(true), attackCooldown(1.0f), timeSinceLastAttack(0.0f), damage(10), startX(startX), startY(startY), isfallsdown(isfallsdown)
    {
        setupMesh();
        texture1 = loadTexture(texturePath);
    }

    void handleMouseClick(GLFWwindow* window, int button, int action, int mods) {
        if (!isAlive) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            float clickX = (2.0f * xpos) / width - 1.0f;
            float clickY = 1.0f - (2.0f * ypos) / height;

            // Adjust click coordinates based on enemy position
            clickX -= x;
            clickY -= y;

            if (clickX >= quadLeft && clickX <= quadRight &&
                clickY >= quadBottom && clickY <= quadTop) {
                // Hit detected, create a bullet trace
                float traceX = x + clickX;  // Convert to world coordinates
                float traceY = y + clickY;
                hp -= 5;
                std::cout << "boss hit! HP: " << hp << std::endl;
                if (hp <= 0) {
                    isAlive = false;
                    std::cout << "par defeated!" << std::endl;
                }
            }
        }
    }

    void addEnemiCollideObject(Character* obj) {
        character = obj;
    }

    bool getIsAlive() const { return isAlive; }

    bool isCollidingWithPlayer() {
        float enemyLeft = x - width / 2;
        float enemyRight = x + width / 2;
        float enemyTop = y + height / 2;
        float enemyBottom = y - height / 2;

        float playerLeft = character->getX() - character->getWidth() / 2;
        float playerRight = character->getX() + character->getWidth() / 2;
        float playerTop = character->getY() + character->getHeight() / 2;
        float playerBottom = character->getY() - character->getHeight() / 2;

        return (enemyLeft < playerRight &&
            enemyRight > playerLeft &&
            enemyTop > playerBottom &&
            enemyBottom < playerTop);
    }

    void attackPlayer() {
        bool isCollidingWithPlayer = this->isCollidingWithPlayer();
        //std::cout << "Enemy position: (" << x << ", " << y << ")" << std::endl;
        //std::cout << "Player position: (" << character->getX() << ", " << character->getY() << ")" << std::endl;
        //std::cout << "Attacking player: cooldown: " << timeSinceLastAttack
        //    << ", colliding: " << isCollidingWithPlayer
        //    << ", distance: " << std::sqrt(std::pow(x - character->getX(), 2) + std::pow(y - character->getY(), 2))
        //    << std::endl;

        if (timeSinceLastAttack >= attackCooldown && isCollidingWithPlayer) {
            character->takeDamage(damage);
            timeSinceLastAttack = 0.0f;
            std::cout << "Attack successful!" << std::endl;
        }
    }


    void setupMesh() {
        float vertices[] = {
            // Positions                                // Colors           // Texture Coords
             characterWidth / 2,  characterHeight / 2, 0.0f, 1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Top Right
             characterWidth / 2, -characterHeight / 2, 0.0f, 0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Bottom Right
            -characterWidth / 2, -characterHeight / 2, 0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Bottom Left
            -characterWidth / 2,  characterHeight / 2, 0.0f, 1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // Top Left 
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

    float getX() const { return x; }
    float getY() const { return y; }

    void move(float dx, float dy, float deltaTime) {
        float newY = y + dy * particleSpeed * deltaTime;
        float newX = x + dx * particleSpeed * deltaTime;
        x = newX;
        y = newY;
    }

    void update(float deltaTime) {
        timeSinceLastAttack += deltaTime;
        // ... другие обновления, если необходимо ...
    }


    void drawParticles() {
        particleShader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1); 
        particleShader.setInt("ourTexture1", 0); 

        glUniform1f(glGetUniformLocation(particleShader.Program, "y_mov"), y);
        glUniform1f(glGetUniformLocation(particleShader.Program, "x_mov"), x);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


    }


    void processInput(GLFWwindow* window, float deltaTime) {
        float dx = 0;
        float dy = 0;

        //std::cout <<"X:  " << character->getX() << ",   " << x << ";  Y: " << character->getY() << ",   " << y << std::endl;

        if (!isfallsdown) {
            dx -= 1.0f;
            if (x < -1.0f || x > 1.0f) {
                isAlive = false;
            }
        }
        else {
            dy -= 1.0f;
            if (y < -1.0f || y > 1.0f) {
                isAlive = false;
            }
        }

        move(dx, dy, deltaTime);
        update(deltaTime);
        attackPlayer();
    }

    void restart() {
            isAlive = true;
            if (!isfallsdown) {
                x = startX;
                y = startY;
            }
            else {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis(-1.0, 0.0);

                double random_number = dis(gen);

                x = random_number;
                y = startY;
            }

            std::cout << "Particle successful!" << std::endl;
        }    
};
#endif // PARTICLE_EMITTER_H