#ifndef ENEMI_H 
#define ENEMI_H  

#include <glad/glad.h> 

#include <vector>
#include <string> 
#include <fstream> 
#include <sstream> 
#include <iostream> 

#include "shader.h"
#include "stb_image.h"
#include "collide.h"
#include "character.h"

#include <GLFW/glfw3.h>


class Enemi {
private:
    float x, y;
    float speed;
    float width, height;
    unsigned int VAO, VBO, EBO;
    unsigned int texture1, texture2;
    Shader shader;
    std::vector<Collide*> collideObjects;

    Character *character;

    float verticalVelocity;
    const float gravity = -1.0f;
    const float jumpStrength = 3.0f;
    bool isOnGround;

    float characterWidth = 0.1f;  // Ширина персонажа в игровом мире
    float characterHeight = 0.5f; // Высота персонажа в игровом мире

    std::vector<Vec4> frames;  // Texture coordinates for each frame
    int currentFrame;
    float frameTime;
    float timeSinceLastFrame;
    bool isMoving;
    bool facingRight;

    float quadLeft, quadRight, quadTop, quadBottom;

    int hp;
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
    Enemi(float startX, float startY, float characterWidth, float characterHeight, float moveSpeed,
        const char* vertexPath, const char* fragmentPath,
        const char* texturePath)
        : x(startX), y(startY), width(characterWidth), height(characterHeight), speed(moveSpeed),
        shader(vertexPath, fragmentPath), verticalVelocity(0.0f), isOnGround(false),
        currentFrame(0), frameTime(0.07f), timeSinceLastFrame(0.0f), isMoving(false), facingRight(true),
        quadLeft(-width / 2), quadRight(width / 2), quadTop(height / 2), quadBottom(-height / 2),
        hp(100), isAlive(true)  // Initialize hp and isAlive
    {
        setupMesh();
        texture1 = loadTexture(texturePath);
        calculateTextureCoords();
    }

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

    void addEnemiCollideObject(Character* obj) {
        character = obj;
    }

    void handleMouseClick(GLFWwindow* window, int button, int action, int mods) {
        if (!isAlive) return;  // Don't process clicks if the enemy is already dead

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
                hp -= 5;
                std::cout << "Enemy hit! HP: " << hp << std::endl;
                if (hp <= 0) {
                    isAlive = false;
                    std::cout << "Enemy defeated!" << std::endl;
                }
            }
        }
    }

    bool getIsAlive() const { return isAlive; }

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

    void draw(float mixValue) {
        shader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        shader.setInt("ourTexture1", 0);

        Vec4 texCoords = frames[currentFrame];
        if (!facingRight) {
            std::swap(texCoords.x, texCoords.z);
        }


        glUniform1f(glGetUniformLocation(shader.Program, "mixValue"), mixValue);
        glUniform1f(glGetUniformLocation(shader.Program, "y_mov"), y);
        glUniform1f(glGetUniformLocation(shader.Program, "x_mov"), x);

        shader.setVec4("texCoords", texCoords.x, texCoords.y, texCoords.z, texCoords.w);


        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void processInput(GLFWwindow* window, float deltaTime) {
        float dx = 0;
        float dy = 0;

        //std::cout <<"X:  " << character->getX() << ",   " << x << ";  Y: " << character->getY() << ",   " << y << std::endl;


        if (character->getX() > x) {
            dx += 1.0f;
        }
        if (character->getX() < x) {
            dx -= 1.0f;
        }
        if (y > (character->getY()-0.8) || y < (character->getY() + 0.8)){
            verticalVelocity = character->getY();
        }
        else if (character->getY() >= y) {
            verticalVelocity += 1.0f;
        }
        if (character->getY() <= y) {
            verticalVelocity -= 0.5f;
        }

        move(dx, dy, deltaTime);
    }

    ~Enemi() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }





};


#endif