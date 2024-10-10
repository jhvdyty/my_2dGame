#include <memory>
#include <stack>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "shader.h"
#include "stb_image.h"
#include "character.h"
#include "collide.h"
#include "enemi.h"
#include "arm.h"
#include "crosshair.h"

class Level1;
class Level2;

// Base class for all game levels
class GameLevel {
protected:
    GLFWwindow* window;
    float lastFrame = 0.0f;

public:
    GameLevel(GLFWwindow* win) : window(win) {}
    virtual ~GameLevel() = default;

    virtual void init() = 0;
    virtual void cleanup() = 0;
    virtual void draw(float deltaTime) = 0;
};

// Game manager singleton
class GameManager {
private:
    static GameManager* instance;
    std::stack<std::unique_ptr<GameLevel>> levels;
    GLFWwindow* window = nullptr; // Fix: Initialize window pointer
    std::vector<Enemi*>* activeEnemies = nullptr;

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        if (instance && instance->activeEnemies) {
            for (auto enemy : *(instance->activeEnemies)) {
                enemy->handleMouseClick(window, button, action, mods);
            }
        }
    }

    GameManager() = default;

public:
    static GameManager* getInstance() {
        if (instance == nullptr) {
            instance = new GameManager();
        }
        return instance;
    }

    void init(GLFWwindow* win) {
        window = win;
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }

    void setActiveEnemies(std::vector<Enemi*>* enemies) {
        activeEnemies = enemies;
    }

    // Fix: Use template to properly handle derived types
    template<typename T>
    void changeLevel(std::unique_ptr<T> level) {
        if (!levels.empty()) {
            levels.top()->cleanup();
            levels.pop();
        }
        level->init();
        levels.push(std::move(level));
    }

    void runGameLoop() {
        float lastFrame = 0.0f;

        while (!glfwWindowShouldClose(window)) {
            float currentFrame = static_cast<float>(glfwGetTime()); // Fix: Explicit cast to float
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            if (!levels.empty()) {
                levels.top()->draw(deltaTime);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
};

// Initialize the static instance
GameManager* GameManager::instance = nullptr;

// Level 2 implementation

class Level2 : public GameLevel {
private:
    std::vector<Enemi*> enemies;
    Character* player;
    Enemi* enemi;
    Collide* ground;
    Collide* platform1;
    Collide* platform2;
    Arm* arm;
    Crosshair* crosshair;

public:
    Level2(GLFWwindow* win) :
        GameLevel(win),
        player(nullptr),
        enemi(nullptr),
        ground(nullptr),
        platform1(nullptr),
        platform2(nullptr),
        arm(nullptr),
        crosshair(nullptr) {}

    void init() override {
        ground = new Collide(0.0f, -0.9f, 2.0f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl");

        player = new Character(
            0.0f, 0.0f, 0.1f, 0.7f, 0.5f,
            "vertex.glsl", "fragment.glsl",
            "texture/character/character.png"
        );

        enemi = new Enemi(
            0.0f, 1.0f, 0.1f, 0.5f, 0.1f,
            "vertex.glsl", "fragment.glsl",
            "texture/brickwall.jpg"
        );

        arm = new Arm(
            0.0f, 0.0f, 0.01f, 0.05f, 2.0f,
            "vertex_arm.glsl", "fragment_arm.glsl",
            "texture/arm.png"
        );

        crosshair = new Crosshair(0.03f);

        // Set up collisions
        player->addCollideObject(ground);

        enemi->addEnemiCollideObject(player);
        enemi->addCollideObject(ground);

        enemies.push_back(enemi);

        arm->addEnemiCollideObject(player);
        arm->addCollideObject(ground);
        arm->addEnemiRotateObject(enemi);

        GameManager::getInstance()->setActiveEnemies(&enemies);
    }

    void cleanup() override {
        GameManager::getInstance()->setActiveEnemies(nullptr);

        delete ground;
        delete player;
        delete arm;
        delete crosshair;

        for (auto enemy : enemies) {
            if (enemy != enemi) {
                delete enemy;
            }
        }
        enemies.clear();

        delete enemi; // Delete enemi after clearing the vector

        ground = nullptr;
        player = nullptr;
        arm = nullptr;
        crosshair = nullptr;
        enemi = nullptr;
    }

    void draw(float deltaTime) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ground->draw();

        arm->processInput(window, deltaTime);
        arm->draw(window, deltaTime);

        player->processInput(window, deltaTime);
        player->draw(window, deltaTime);

        for (auto it = enemies.begin(); it != enemies.end();) {
            if ((*it)->getIsAlive()) {
                (*it)->processInput(window, deltaTime);
                (*it)->draw();
                ++it;
            }
            else {
                if (*it != enemi) {
                    delete* it;
                }
                it = enemies.erase(it);
            }
        }

        crosshair->draw(window);

        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            GameManager::getInstance()->changeLevel<Level1>(
                std::make_unique<Level1>(window)
            );
        }


    }

    ~Level2() {
        cleanup();
    }
};


class Level1 : public GameLevel {
private:
    std::vector<Enemi*> enemies;
    Character* player;
    Enemi* enemi;
    Collide* ground;
    Collide* platform1;
    Collide* platform2;
    Arm* arm;
    Crosshair* crosshair;

public:
    Level1(GLFWwindow* win) :
        GameLevel(win),
        player(nullptr),
        enemi(nullptr),
        ground(nullptr),
        platform1(nullptr),
        platform2(nullptr),
        arm(nullptr),
        crosshair(nullptr) {}

    void init() override {
        ground = new Collide(0.0f, -0.9f, 2.0f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl");
        platform1 = new Collide(0.3f, -0.5f, 0.5f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl");
        platform2 = new Collide(-0.4f, -0.08f, 0.5f, 0.1f, 1.0f, "vertex_full.glsl", "fragment_full.glsl");

        player = new Character(
            0.0f, 0.0f, 0.1f, 0.7f, 0.5f,
            "vertex.glsl", "fragment.glsl",
            "texture/character/character.png"
        );

        enemi = new Enemi(
            0.0f, 1.0f, 0.1f, 0.5f, 0.1f,
            "vertex.glsl", "fragment.glsl",
            "texture/brickwall.jpg"
        );

        arm = new Arm(
            0.0f, 0.0f, 0.01f, 0.05f, 2.0f,
            "vertex_arm.glsl", "fragment_arm.glsl",
            "texture/arm.png"
        );

        crosshair = new Crosshair(0.03f);

        // Set up collisions
        player->addCollideObject(ground);
        player->addCollideObject(platform1);
        player->addCollideObject(platform2);

        enemi->addEnemiCollideObject(player);
        enemi->addCollideObject(ground);
        enemi->addCollideObject(platform1);
        enemi->addCollideObject(platform2);

        enemies.push_back(enemi);

        arm->addEnemiCollideObject(player);
        arm->addCollideObject(ground);
        arm->addCollideObject(platform1);
        arm->addCollideObject(platform2);
        arm->addEnemiRotateObject(enemi);

        GameManager::getInstance()->setActiveEnemies(&enemies);
    }

    void cleanup() override {
        GameManager::getInstance()->setActiveEnemies(nullptr);

        delete ground;
        delete platform1;
        delete platform2;
        delete player;
        delete arm;
        delete crosshair;

        for (auto enemy : enemies) {
            if (enemy != enemi) {
                delete enemy;
            }
        }
        enemies.clear();

        delete enemi; // Delete enemi after clearing the vector

        ground = nullptr;
        platform1 = nullptr;
        platform2 = nullptr;
        player = nullptr;
        arm = nullptr;
        crosshair = nullptr;
        enemi = nullptr;
    }

    void draw(float deltaTime) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ground->draw();
        platform1->draw();
        platform2->draw();

        arm->processInput(window, deltaTime);
        arm->draw(window, deltaTime);

        player->processInput(window, deltaTime);
        player->draw(window, deltaTime);

        for (auto it = enemies.begin(); it != enemies.end();) {
            if ((*it)->getIsAlive()) {
                (*it)->processInput(window, deltaTime);
                (*it)->draw();
                ++it;
            }
            else {
                if (*it != enemi) {
                    delete* it;
                }
                it = enemies.erase(it);
            }
        }

        crosshair->draw(window);

        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            GameManager::getInstance()->changeLevel<Level2>(
                std::make_unique<Level2>(window)
            );
        }

    }

    ~Level1() {
        cleanup();
    }
};



// Main menu implementation
class MainMenu : public GameLevel {
public:
    MainMenu(GLFWwindow* win) : GameLevel(win) {}

    void init() override {
        // Initialize menu components
    }

    void cleanup() override {
        // Cleanup menu resources
    }

    void draw(float deltaTime) override {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw menu components

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            GameManager::getInstance()->changeLevel<Level1>(
                std::make_unique<Level1>(window)
            );
        }
    }
};