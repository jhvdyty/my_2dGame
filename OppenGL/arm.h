#ifndef ARM_H 
#define ARM_H  

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#include <glad/glad.h> 

#include <cmath>

#include <vector>
#include <string> 
#include <fstream> 
#include <sstream> 
#include <iostream> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "stb_image.h"
#include "collide.h"
#include "character.h"
#include "enemi.h"

#include <GLFW/glfw3.h>




class Arm {
private:
    float x, y;
    float speed;
    float width, height;
    unsigned int VAO, VBO, EBO;
    unsigned int VAO_vertical, VAO_horizontal;
    unsigned int VBO_vertical, VBO_horizontal;
    unsigned int EBO_vertical, EBO_horizontal;

    unsigned int texture1, texture2;
    Shader shader;
    std::vector<Collide*> collideObjects;

    Character* character;
    Enemi* enemi;

    float verticalVelocity;
    const float gravity = -1.0f;
    const float jumpStrength = 3.0f;
    bool isOnGround;

    float characterWidth = 0.25f;  // Øèðèíà ïåðñîíàæà â èãðîâîì ìèðå
    float characterHeight = 0.65f; // Âûñîòà ïåðñîíàæà â èãðîâîì ìèðå

    std::vector<Vec4> frames;  // Texture coordinates for each frame
    int currentFrame;
    float frameTime;
    float timeSinceLastFrame;
    bool isMoving;
    bool facingRight;

    float quadLeft, quadRight, quadTop, quadBottom;

    int hp;
    bool isAlive;

    float angel;

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
    Arm(float startX, float startY, float characterWidth, float characterHeight, float moveSpeed,
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
    void addEnemiRotateObject(Enemi* obj) {
        enemi = obj;
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
        // Âåðòèêàëüíîå ñîñòîÿíèå
        float vertices_vertical[] = {
            // Positions                              // Colors           // Texture Coords
              0.7f,  0.37f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Top Right
              0.7f, -0.37f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Bottom Right
             -0.7f, -0.37f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Bottom Left
             -0.7f,  0.37f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // Top Left 
        };

        // Ãîðèçîíòàëüíîå ñîñòîÿíèå
        float vertices_horizontal[] = {
            // Positions                              // Colors           // Texture Coords
              0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Top Right
              0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Bottom Right
             -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Bottom Left
             -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // Top Left 
        };

        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        // Íàñòðîéêà âåðòèêàëüíîãî VAO
        glGenVertexArrays(1, &VAO_vertical);
        glGenBuffers(1, &VBO_vertical);
        glGenBuffers(1, &EBO_vertical);

        glBindVertexArray(VAO_vertical);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_vertical);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_vertical), vertices_vertical, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_vertical);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Íàñòðîéêà ãîðèçîíòàëüíîãî VAO
        glGenVertexArrays(1, &VAO_horizontal);
        glGenBuffers(1, &VBO_horizontal);
        glGenBuffers(1, &EBO_horizontal);

        glBindVertexArray(VAO_horizontal);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_horizontal);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_horizontal), vertices_horizontal, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_horizontal);
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

        isMoving = (character->getDX() != 0);
        if (character->getDX() > 0) facingRight = true;
        else if (character->getDX() < 0) facingRight = false;

        //isMoving = (dx != 0);
        //if (dx > 0) facingRight = true;
        //else if (dx < 0) facingRight = false;

        updateAnimation(deltaTime);
    }


    void jump() {
        if (isOnGround) {
            verticalVelocity = jumpStrength;
            isOnGround = false;
        }
    }

    void ro_calcul(float posx, float posy) {
        //std::cout << "maus x,y: " << posx << " " << posy << ";    player x,y:" << x << " " << y << std::endl;
        angel = std::atan2(posy, posx);
        angel -= 1.5f;
    }

    void draw(GLFWwindow* window, float mixValue, float deltaTime) {
        shader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        shader.setInt("ourTexture1", 0);

        Vec4 texCoords = frames[currentFrame];
        if (!facingRight) {
            std::swap(texCoords.x, texCoords.z);
        }
        //std::cout << "facingRight: " << facingRight << ";     isMoving: " << isMoving << std::endl;


        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float clickX = (2.0f * xpos) / width - 1.0f;
        float clickY = 1.0f - (2.0f * ypos) / height;

        // Adjust click coordinates based on enemy position
        clickX -= x;
        clickY -= y;

        ro_calcul(clickX, clickY);

        float angle_normalized = fmod(angel, 2 * M_PI);
        if (angle_normalized < 0) angle_normalized += 2 * M_PI;

        bool use_vertical = (angle_normalized > M_PI / 4 && angle_normalized < 3 * M_PI / 4) ||
            (angle_normalized > 5 * M_PI / 4 && angle_normalized < 7 * M_PI / 4);

        float pi = M_PI;

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
        transform = glm::rotate(transform, angel, glm::vec3(0.0f, 0.0f, 1.0f));

        transform = glm::scale(transform, glm::vec3(characterWidth, characterHeight, 1.0f));


        // Âðåìåííî îòêëþ÷èì âðàùåíèå äëÿ îòëàäêè
        // transform = glm::rotate(transform, (GLfloat)glfwGetTime() * 50.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        // Óñòàíîâêà uniform-ïåðåìåííûõ
        GLint transformLoc = glGetUniformLocation(shader.Program, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        //glUniform1f(glGetUniformLocation(shader.Program, "mixValue"), mixValue);
        //glUniform1f(glGetUniformLocation(shader.Program, "y_mov"), y);
        //glUniform1f(glGetUniformLocation(shader.Program, "x_mov"), x);
        //glUniform1f(glGetUniformLocation(shader.Program, "y_target"), xpos);
        //glUniform1f(glGetUniformLocation(shader.Program, "x_target"), ypos);

        shader.setVec4("texCoords", texCoords.x, texCoords.y, texCoords.z, texCoords.w);


        glBindVertexArray(use_vertical ? VAO_vertical : VAO_horizontal);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }



    void processInput(GLFWwindow* window, float deltaTime) {
        float dx = 0;
        float dy = 0;

        //std::cout <<"X:  " << character->getX() << ",   " << x << ";  Y: " << character->getY() << ",   " << y << std::endl;

        if (x > (character->getX() - 0.8) || x < (character->getX() + 0.8)) {
            x = character->getX();
        }
        else if (character->getX() > x) {
            dx += 1.0f;
        }
        if (character->getX() < x) {
            dx -= 1.0f;
        }
        if (y > (character->getY() - 0.8) || y < (character->getY() + 0.8)) {
            verticalVelocity = character->getY();
        }
        else if (character->getY() >= y) {
            y = character->getY() + 0.19f; //0.251f
        }
        if (character->getY() <= y) {
            y = character->getY() + 0.19f; //0.251f
        }

        move(dx, dy, deltaTime);
    }

    ~Arm() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }





};


#endif