my_2dGame
A 2D side-scrolling game built using C++, OpenGL, and GLFW. This project features basic platformer mechanics, two playable levels with enemy AI, shooting mechanics, particle effects, and a transition system between levels.

Features
Two main levels (Level1 and Level2) with seamless transitions

Enemy AI with interactions and health

Basic physics and collisions (Collide system)

Particle effects and boss fights

Arm weapon system with aiming and shooting

Crosshair-controlled input

Game state management using a custom stack-based engine

Modular and extensible game architecture

🗂 Project Structure
graphql
Копировать код
my_2dGame/
├── OpenGL/
│   ├── GameState.h          # Core game state management (levels, transitions)
│   ├── shader.h             # Shader loading and compilation
│   ├── character.h          # Player logic and rendering
│   ├── collide.h            # Platform and ground collision handling
│   ├── enemi.h              # Enemy and boss behavior
│   ├── arm.h                # Weapon/arm aiming and shooting
│   ├── crosshair.h          # Cursor handling
│   └── stb_image.h          # Image loading
├── shaders/
│   ├── vertex.glsl, fragment.glsl, ...  # Various shader programs
├── texture/
│   ├── wall.jpeg, character.png, enemi_texture.png, ...
└── main.cpp                 # Entry point for the application
Architecture
GameManager: Singleton managing the game loop and level transitions

GameLevel (abstract): Base class for all levels (MainMenu, Level1, Level2)

Level1 / Level2: Implementations of levels with unique enemies, layout, and behavior

MainMenu: Start screen that switches to Level1 on spacebar press

Entities: Player, enemies, arm, crosshair, and interactive objects like ParticleEmitter

🛠 Dependencies
Make sure you have the following installed:

GLFW

GLAD

stb_image

OpenGL 3.3+

C++17 or later

How to Build & Run
Clone the repo

bash
Копировать код
git clone https://github.com/yourusername/my_2dGame.git
cd my_2dGame
Install dependencies
Ensure glfw, glad, and OpenGL libraries are properly linked. If using Linux:

bash
Копировать код
sudo apt install libglfw3-dev libglew-dev libglm-dev
Build the game

bash
Копировать код
g++ main.cpp -I./OpenGL -lglfw -ldl -lGL -o my_2dGame
Or use a CMake project file if available.

Run

bash
./my_2dGame
Controls
Arrow keys or WASD: Move the player

Mouse: Aim and interact

Left-click: Shoot or interact with enemies

Spacebar: Start the game (from main menu)

Player transitions between levels when reaching screen bounds or falling/dying.

Screenshots
![image](https://github.com/user-attachments/assets/51b3c934-804a-42c8-8963-0bfbeed4795a)
![image](https://github.com/user-attachments/assets/36a2f6e3-429a-4f93-8d91-72444ebe2e3a)
![image](https://github.com/user-attachments/assets/3493f0a0-6ab9-4bdf-b09a-3c86a18d6337)


License
This project is open-source and free to use. See the LICENSE file for details.
