#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "stb_image.h"
#include "character.h"
#include "collide.h"
#include "enemi.h"
#include "arm.h"
#include "crosshair.h"

float lastFrame = 0.0f; // Время последнего кадра


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLfloat mixValue = 0.2f;

std::vector<Enemi*> enemies;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mixValue += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mixValue -= 0.01f;
}

void global_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    for (auto enemy : enemies) {
        enemy->handleMouseClick(window, button, action, mods);
    }
}


int main() {
    glfwInit();



    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(1903, 911, "Hello Window!", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


    Collide ground(0.0f, -0.9f, 2.0f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl"); 
    Collide platform1(0.3f, -0.5f, 0.5f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl"); 
    Collide platform2(-0.4f, -0.08f, 0.5f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl"); 


    Character player( 
         0.0f, 0.0f,0.1f, 0.7f, 0.5f,
        "vertex.glsl", "fragment.glsl",
        "texture/character/character.png"
    );

    Enemi enemi(
        0.0f, 0.0f, 0.1f, 0.5f, 0.1f,
        "vertex.glsl", "fragment.glsl",
        "texture/brickwall.jpg"
    );

    Arm arm(
        0.0f, 0.0f, 0.01f, 0.05f, 2.0f,
        "vertex_arm.glsl", "fragment_arm.glsl",
        "texture/arm.png" //
    );

    Crosshair crosshair(0.03f);

    player.addCollideObject(&ground); 
    player.addCollideObject(&platform1); 
    player.addCollideObject(&platform2); 

    enemi.addEnemiCollideObject(&player);
    enemi.addCollideObject(&ground);
    enemi.addCollideObject(&platform1);
    enemi.addCollideObject(&platform2);

    enemies.push_back(&enemi);

    arm.addEnemiCollideObject(&player);
    arm.addCollideObject(&ground);
    arm.addCollideObject(&platform1);
    arm.addCollideObject(&platform2);
    arm.addEnemiRotateObject(&enemi);


    glfwSetMouseButtonCallback(window, global_mouse_button_callback); 


    //glVertexAttribPointer(1, 2, GL_FLOAT, GLFW_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
 

    while (!glfwWindowShouldClose(window)) {
        // ... Обработка событий (если необходимо) ...

        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame; 
        lastFrame = currentFrame; 

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // ... Команды отрисовки ...

        //GLfloat timeValue = glfwGetTime();
        //GLfloat greenValue = (sin(timeValue) / 2) + 0.5;
        //GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");

        ground.draw();
        platform1.draw();
        platform2.draw();

        arm.processInput(window, deltaTime);
        arm.draw(window, mixValue, deltaTime);

        processInput(window);
        player.processInput(window, deltaTime);
        player.draw(window, mixValue, deltaTime);

        for (auto it = enemies.begin(); it != enemies.end();) {
            if ((*it)->getIsAlive()) {
                (*it)->processInput(window, deltaTime);
                (*it)->draw(mixValue);
                ++it;
            }
            else {
                // Remove dead enemy from the vector
                it = enemies.erase(it);
            }
        }

        crosshair.draw(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
