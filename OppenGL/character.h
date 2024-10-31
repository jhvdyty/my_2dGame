#ifndef CHARACTER_H
#define CHARACTER_H

#include <glad/glad.h> 

#include <vector>
#include <string> 
#include <fstream> 
#include <sstream> 
#include <iostream> 

#include "shader.h"
#include "stb_image.h"
#include "collide.h"

#include <GLFW/glfw3.h>

class Character;

struct Vec4 {
    float x, y, z, w;
    Vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}
};

class Character {
private:
    float x, y;
    float speed;
    float width, height;
    unsigned int VAO, VBO, EBO;
    unsigned int texture1, texture2;
    Shader shader;
    std::vector<Collide*> collideObjects;

    float verticalVelocity;
    const float gravity = -9.8f;
    const float jumpStrength = 3.5f;
    bool isOnGround;

    float characterWidth = 0.2f;  // Ширина персонажа в игровом мире
    float characterHeight = 0.55f; // Высота персонажа в игровом мире 

    std::vector<Vec4> frames;  // Texture coordinates for each frame
    int currentFrame;
    float frameTime;
    float timeSinceLastFrame;
    bool isMoving;
    bool facingRight;

    int hp;
    float invincibilityTime;
    float timeSinceLastHit;
    bool isAlive;

    unsigned int loadTexture(const char* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);



        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

        int width, height, nrChannels;
        unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            int framesPerRow = 4;  // Adjust based on your spritesheet
            int framesPerColumn = 4;
            float frameWidth = 1.0f / framesPerRow;
            float frameHeight = 1.0f / framesPerColumn;

            for (int y = 0; y < framesPerColumn; ++y) {
                for (int x = 0; x < framesPerRow; ++x) {
                    frames.push_back(Vec4(
                        x * frameWidth, y * frameHeight,
                        (x + 1) * frameWidth, (y + 1) * frameHeight
                    ));
                }
            }
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        return textureID;
    }

    void calculateTextureCoords() {
        int framesPerRow = 4;
        int framesPerColumn = 2;  // Изменено на 2, так как у нас 2 ряда кадров
        float frameWidth = 1.0f / framesPerRow;
        float frameHeight = 1.0f / framesPerColumn;

        frames.clear();
        for (int y = 0; y < framesPerColumn; ++y) {
            for (int x = 0; x < framesPerRow; ++x) {
                frames.push_back(Vec4(
                    x * frameWidth,
                    1.0f - (y + 1) * frameHeight,  // Инвертируем Y-координату
                    (x + 1) * frameWidth,
                    1.0f - y * frameHeight  // Инвертируем Y-координату
                ));
            }
        }
    }


public:
    Character(float startX, float startY, float characterWidth, float characterHeight, float moveSpeed,
        const char* vertexPath, const char* fragmentPath,
        const char* texturePath)
        : x(startX), y(startY), width(characterWidth), height(characterHeight), speed(moveSpeed),
        shader(vertexPath, fragmentPath), verticalVelocity(0.0f), isOnGround(false),
        currentFrame(0), frameTime(0.07f), timeSinceLastFrame(0.0f), isMoving(false), facingRight(true),
        hp(100), invincibilityTime(1.0f), timeSinceLastHit(0.0f), isAlive(true) // Инициализация новых членов
    {
        setupMesh();
        texture1 = loadTexture(texturePath);
        calculateTextureCoords();
    }

    void setPosition(float x, float y) {
        this->x = x;
        this->y = y;
    }

    void takeDamage(int damage) {
        if (timeSinceLastHit >= invincibilityTime) {
            hp -= damage;
            timeSinceLastHit = 0.0f;
            std::cout << "Player took " << damage << " damage. Remaining HP: " << hp << std::endl;
            if (hp <= 0) {
                std::cout << "Player defeated!" << std::endl;
                // Здесь можно добавить логику окончания игры или респавна
                isAlive = false;
            }
        }
    }
    bool getIsAlive() const { return isAlive; }
    int getHP() const { return hp; }

    float getWidth() const { return width; }
    float getHeight() const { return height; }



    void updateAnimation(float deltaTime) {
        if (isMoving) {
            timeSinceLastFrame += deltaTime;
            if (timeSinceLastFrame >= frameTime) {
                currentFrame = (currentFrame + 1) % 8;  // Предполагаем, что у нас 8 кадров анимации
                timeSinceLastFrame = 0.0f;
            }
        }
        else {
            currentFrame = 0;  // Кадр покоя
        }
    }

    void addCollideObject(Collide* obj) {
        collideObjects.push_back(obj);
    }

    bool isColliding(float newX, float newY) {
        for (const auto& obj : collideObjects) {
            if (newX + width / 2 >= obj->getX() - obj->getWidth() / 2 &&
                newX - width / 2 <= obj->getX() + obj->getWidth() / 2) {
                if (newY + height / 2 >= obj->getY() - obj->getHeight() / 2 &&
                    newY - height / 2 <= obj->getY() + obj->getHeight() / 2) {
                    return true; // Collision detected
                }
            }
        }


        return false; // No collision
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
        float newX = x + dx * speed * deltaTime;
        float newY = y + verticalVelocity * deltaTime;

        verticalVelocity += gravity * deltaTime;

        bool collidedVertically = false;
        for (const auto& obj : collideObjects) {
            if (newX + width / 2 >= obj->getX() - obj->getWidth() / 2 &&
                newX - width / 2 <= obj->getX() + obj->getWidth() / 2) {
                if (newY - height / 2 <= obj->getY() + obj->getHeight() / 2 &&
                    y - height / 2 > obj->getY() + obj->getHeight() / 2) {
                    // Landing on top of an object
                    newY = obj->getY() + obj->getHeight() / 2 + height / 2;
                    verticalVelocity = 0;
                    isOnGround = true;
                    collidedVertically = true;
                    break;
                }
            }
        }

        if (!collidedVertically) {
            isOnGround = false;
        }

        if (!isColliding(newX, y)) {
            x = newX;
        }

        if (!isColliding(x, newY)) {
            y = newY;
        }
        else if (verticalVelocity < 0) {
            // If we're moving down and collide, stop vertical movement
            verticalVelocity = 0;
            isOnGround = true;
        }
         
        isMoving = (dx != 0); 
        if (dx > 0) facingRight = true; 
        else if (dx < 0) facingRight = false; 

        updateAnimation(deltaTime); 
    }


    void jump() {
        if (isOnGround) {
            verticalVelocity = jumpStrength;
            isOnGround = false;
        }
    }

    void update(float deltaTime) {
        timeSinceLastHit += deltaTime;
        // ... другие обновления, если необходимо ...
    }

    void draw(GLFWwindow* window, float deltaTime) {

        shader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        shader.setInt("ourTexture1", 0);

        Vec4 texCoords = frames[currentFrame];
        if (!facingRight) {
            std::swap(texCoords.x, texCoords.z);
        }

        glUniform1f(glGetUniformLocation(shader.Program, "y_mov"), y);
        glUniform1f(glGetUniformLocation(shader.Program, "x_mov"), x);

        shader.setVec4("texCoords", texCoords.x, texCoords.y, texCoords.z, texCoords.w);


        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    float dx = 0;

    void processInput(GLFWwindow* window, float deltaTime) {
        dx = 0;

        //std::cout << "X: " << x << ";      Y:" << y << std::endl;
         
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { 
            dx -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { 
            dx += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            jump();
        }

        move(dx, 0, deltaTime);
    }
    float getDX() const { return dx; }

    ~Character() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }





};




#endif