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
    static GameLevel* currentLevel; // Add static pointer to current level

public:
    GameLevel(GLFWwindow* win) : window(win) {}
    virtual ~GameLevel() = default;

    virtual void init() = 0;
    virtual void cleanup() = 0;
    virtual void draw(float deltaTime) = 0;
    virtual void handleMouseClick(GLFWwindow* window, int button, int action, int mods) = 0;

    static void setCurrentLevel(GameLevel* level) {
        currentLevel = level;
    }

    static void globalMouseCallback(GLFWwindow* window, int button, int action, int mods) {
        if (currentLevel) {
            currentLevel->handleMouseClick(window, button, action, mods);
        }
    }
};

GameLevel* GameLevel::currentLevel = nullptr;

// Game manager singleton
class GameManager {
private:
    static GameManager* instance;
    std::stack<std::unique_ptr<GameLevel>> levels;
    GLFWwindow* window = nullptr;

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
        glfwSetMouseButtonCallback(window, GameLevel::globalMouseCallback);
    }

    template<typename T>
    void changeLevel(std::unique_ptr<T> level) {
        if (!levels.empty()) {
            levels.top()->cleanup();
            levels.pop();
        }
        level->init();
        GameLevel::setCurrentLevel(level.get()); // Set current level before pushing
        levels.push(std::move(level));
    }

    void runGameLoop() {
        float lastFrame = 0.0f;

        while (!glfwWindowShouldClose(window)) {
            float currentFrame = static_cast<float>(glfwGetTime());
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
    Character* player;
    Enemi* enemi;
    Collide* ground;
    Collide* platform1;
    Collide* platform2;
    Arm* arm;
    Crosshair* crosshair;
    Boss* boss;
    ParticleEmitter* particle;
    ParticleEmitter* fallparticle;
    float particleCooldown = 5.0f, timeSinceLastParticle = 0.0f;
    bool change = false;

public:
    Level2(GLFWwindow* win) :
        GameLevel(win),
        player(nullptr),
        enemi(nullptr),
        boss(nullptr),
        ground(nullptr),
        platform1(nullptr),
        platform2(nullptr),
        arm(nullptr),
        particle(nullptr),
        fallparticle(nullptr),
        crosshair(nullptr) {}


    void handleMouseClick(GLFWwindow* window, int button, int action, int mods) override {
        if (enemi) {
            enemi->handleMouseClick(window, button, action, mods);
        }
        if (boss) {
            boss->handleMouseClick(window, button, action, mods);
        }
        if (particle) {
            particle->handleMouseClick(window, button, action, mods);
        }
        if (fallparticle) {
            fallparticle->handleMouseClick(window, button, action, mods);
        }
    }

    void init() override {

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        ground = new Collide(
            0.0f, -0.9f, 2.0f, 0.1f, 1.0f,
            "vertex_full.glsl",
            "fragment_full.glsl",
            "texture/wall.jpeg"
        );

        player = new Character(
            -0.9f, 0.0f, 0.1f, 0.55f, 0.5f,
            "vertex.glsl", "fragment.glsl",
            "texture/character/character.png"
        );

        enemi = new Enemi(
            0.0f, 1.0f, 0.09f, 0.35f, 0.1f,
            "vertex.glsl", "fragment.glsl",
            "texture/enemi_texture.png",
            "vertex_BulletTrace.glsl", "fragment_BulletTrace.glsl"
        );
        boss = new Boss(
            0.0f, -0.5f, 0.09f, 0.35f, 0.0f,
            "vertex.glsl", "fragment.glsl",
            "texture/enemi_texture.png",
            "vertex_BulletTrace.glsl", "fragment_BulletTrace.glsl",
            width, height
        );

        arm = new Arm(
            0.0f, 0.0f, 0.01f, 0.05f, 2.0f,
            "vertex_arm.glsl", "fragment_arm.glsl",
            "texture/arm.png"
        );

        particle = new ParticleEmitter(
            0.0f, -0.5f, 0.09f, 0.35f, 0.15f,
            "vertex_particle.glsl", "fragment_particle.glsl",
            "texture/wall.jpeg",
            width, height, false
        );

        fallparticle = new ParticleEmitter(
            0.0f, 0.7f, 0.09f, 0.35f, 0.15f,
            "vertex_particle.glsl", "fragment_particle.glsl",
            "texture/wall.jpeg",
            width, height, true 
        );

        crosshair = new Crosshair(0.03f);

        // Set up collisions
        player->addCollideObject(ground);

        enemi->addEnemiCollideObject(player);
        enemi->addCollideObject(ground);

        boss->addEnemiCollideObject(player);
        boss->addCollideObject(ground);

        particle->addEnemiCollideObject(player);

        fallparticle->addEnemiCollideObject(player);

        arm->addEnemiCollideObject(player);
        arm->addCollideObject(ground);
        arm->addEnemiRotateObject(enemi);
        arm->addEnemiRotateObject(boss);
    }

    void cleanup() override {
        // Сначала удаляем объекты, которые зависят от других объектов
        if (arm) {
            delete arm;
            arm = nullptr;
        }

        if (crosshair) {
            delete crosshair;
            crosshair = nullptr;
        }

        // Удаляем врагов
        if (enemi) {
            delete enemi;
            enemi = nullptr;
        }

        if (boss) {
            delete boss;
            boss = nullptr;
        }

        //Удаляем частицы только если босс мертв или происходит смена уровня
        if ((!boss || !boss->getIsAlive() || change) && particle) {
            delete particle;
            particle = nullptr;
            change = false;
        }

        if ((!boss || !boss->getIsAlive() || change) && fallparticle) {
            delete fallparticle;
            fallparticle = nullptr;
            change = false;
        }

        //delete particle;
        //particle = nullptr;

        // Удаляем игрока
        if (player) {
            delete player;
            player = nullptr;
        }

        // Удаляем коллайдеры
        if (ground) {
            delete ground;
            ground = nullptr;
        }

        if (platform1) {
            delete platform1;
            platform1 = nullptr;
        }

        if (platform2) {
            delete platform2;
            platform2 = nullptr;
        }
    }

    void draw(float deltaTime) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        timeSinceLastParticle += deltaTime;

        ground->draw();

        arm->processInput(window, deltaTime);
        arm->draw(window, deltaTime);

        player->processInput(window, deltaTime);
        player->draw(window, deltaTime);
        player->update(deltaTime);

        if (enemi && enemi->getIsAlive()) {
            enemi->processInput(window, deltaTime);
            enemi->draw(deltaTime);
        }

        if (boss && boss->getIsAlive()) {
            boss->processInput(window, deltaTime);
            boss->draw(deltaTime);
        }

        if (fallparticle && fallparticle->getIsAlive()) {
            fallparticle->processInput(window, deltaTime);
            fallparticle->drawParticles();
        }
        else if (timeSinceLastParticle >= particleCooldown && boss && boss->getIsAlive() && fallparticle && !fallparticle->getIsAlive()) {
            timeSinceLastParticle = 0.0f;
            fallparticle->restart();
        
        }

        if (particle && particle->getIsAlive()) {
            particle->processInput(window, deltaTime);
            particle->drawParticles();
        }
        else if (timeSinceLastParticle >= particleCooldown && boss && boss->getIsAlive() && particle && !particle->getIsAlive()) {
            timeSinceLastParticle = 0.0f;
            particle->restart();

        }

        crosshair->draw(window);

        if (player->getX() < -1.0f) {
            change = true;
            GameManager::getInstance()->changeLevel<Level1>(
                std::make_unique<Level1>(window)
            );
        }
        else if (player->getY() < -2.0f) {
            change = true;
            GameManager::getInstance()->changeLevel<Level2>(
                std::make_unique<Level2>(window)
            );
        }
        else if (!player->getIsAlive()) {
            change = true;
            GameManager::getInstance()->changeLevel<Level2>(
                std::make_unique<Level2>(window)
            );
        }


    }

    ~Level2() {
        cleanup();
    }
};


class Level1 : public GameLevel {
private:
    Character* player;
    Enemi* enemi;
    Enemi* enemi2;
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
        enemi2(nullptr),
        ground(nullptr),
        platform1(nullptr),
        platform2(nullptr),
        arm(nullptr),
        crosshair(nullptr) {}


    void handleMouseClick(GLFWwindow* window, int button, int action, int mods) override {
        if (enemi) {
            enemi->handleMouseClick(window, button, action, mods);
        }
        if (enemi2) {
            enemi2->handleMouseClick(window, button, action, mods);
        }
    }

    void init() override {
        ground = new Collide(
            0.0f, -0.9f, 2.0f, 0.1f, 1.0f,
            "vertex_full.glsl",
            "fragment_full.glsl",
            "texture/wall.jpeg"
        );
        platform1 = new Collide(
            0.3f, -0.5f, 0.5f, 0.1f, 1.0f,
            "vertex_full.glsl",
            "fragment_full.glsl",
            "texture/wall.jpeg"
        );
        platform2 = new Collide(
            -0.4f, -0.08f, 0.5f, 0.1f, 1.0f,
            "vertex_full.glsl",
            "fragment_full.glsl",
            "texture/wall.jpeg"
        );

        player = new Character(
            0.9f, 0.0f, 0.1f, 0.55f, 0.5f,
            "vertex.glsl", "fragment.glsl",
            "texture/character/character.png"
        );

        enemi = new Enemi(
            0.0f, 1.0f, 0.09f, 0.35f, 0.1f,
            "vertex.glsl", "fragment.glsl",
            "texture/enemi_texture.png",
            "vertex_BulletTrace.glsl", "fragment_BulletTrace.glsl"
        );

        enemi2 = new Enemi(
            0.0f, 1.0f, 0.09f, 0.35f, 0.3f,
            "vertex.glsl", "fragment.glsl",
            "texture/enemi_texture.png",
            "vertex_BulletTrace.glsl", "fragment_BulletTrace.glsl"
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

        enemi2->addEnemiCollideObject(player);
        enemi2->addCollideObject(ground);
        enemi2->addCollideObject(platform1);
        enemi2->addCollideObject(platform2);

        arm->addEnemiCollideObject(player);
        arm->addCollideObject(ground);
        arm->addCollideObject(platform1);
        arm->addCollideObject(platform2);
        arm->addEnemiRotateObject(enemi);
        arm->addEnemiRotateObject(enemi2);

    }

    void cleanup() override {
        // Сначала удаляем объекты, которые зависят от других объектов
        if (arm) {
            delete arm;
            arm = nullptr;
        }

        if (crosshair) {
            delete crosshair;
            crosshair = nullptr;
        }

        // Удаляем врагов
        if (enemi) {
            delete enemi;
            enemi = nullptr;
        }

        if (enemi2) {
            delete enemi2;
            enemi2 = nullptr;
        }

        // Удаляем игрока
        if (player) {
            delete player;
            player = nullptr;
        }

        // Удаляем коллайдеры
        if (ground) {
            delete ground;
            ground = nullptr;
        }

        if (platform1) {
            delete platform1;
            platform1 = nullptr;
        }

        if (platform2) {
            delete platform2;
            platform2 = nullptr;
        }
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
        player->update(deltaTime);


        if (enemi && enemi->getIsAlive()) {
            enemi->processInput(window, deltaTime);
            enemi->draw(deltaTime);
        }

        if (enemi2 && enemi2->getIsAlive()) {
            enemi2->processInput(window, deltaTime);
            enemi2->draw(deltaTime);
        }

        crosshair->draw(window);

        if (player->getX() > 1.0f) {
            GameManager::getInstance()->changeLevel<Level2>(
                std::make_unique<Level2>(window)
            );
        }
        else if (player->getY() < -2.0f) {
            GameManager::getInstance()->changeLevel<Level1>(
                std::make_unique<Level1>(window)
            );
        }
        else if (!player->getIsAlive()) {
            GameManager::getInstance()->changeLevel<Level1>(
                std::make_unique<Level1>(window)
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

    void handleMouseClick(GLFWwindow* window, int button, int action, int mods) override {
        // Handle menu mouse clicks if needed
    }

    void init() override {
        // Initialize menu components
    }

    void cleanup() override {
        // Cleanup menu resources
    }

    void draw(float deltaTime) override {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            GameManager::getInstance()->changeLevel<Level1>(
                std::make_unique<Level1>(window)
            );
        }
    }
};