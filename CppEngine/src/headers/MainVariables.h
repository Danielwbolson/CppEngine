
// Holds main function variables and include statements

#include <SDL.h>
#include <ostream>
#include "glad/glad.h"
#include "SDL_Static_Helper.h"

#include "Systems.h"
#include "RendererSystem.h"
#include "ColliderSystem.h"
#include "PhysicsSystem.h"

#include "Globals.h"
#include "Camera.h"
#include "Scene.h"
#include "AssetManager.h"
#include "MemoryManager.h"

#include "Configuration.h"

#include "lua-5.3.5/src/lua.hpp"
#include "LuaSupport.h"
#include "sol/sol.hpp"

const std::string sceneFile = "scene.txt";
const std::string gameObjectsFile = "gameObjects.txt";
std::string luaMain = "main.lua";
std::string gameFolder = "";

std::ofstream performanceStream;

sol::state lua;
sol::function frameUpdate;
sol::function keyInput;

int windowWidth = 1200;
int windowHeight = 800;
int engineComponentsSize = 3;

float luaTimeSpeed = 1;
float startTime, printTime;
bool quit = false;

std::vector<Systems*> systems;

// SDL objects
SDL_Window* window;
SDL_GLContext context;

// Globals
Camera* mainCamera;
Scene* mainScene;
AssetManager* assetManager;
MemoryManager* memoryManager;

// Helper functions
int Init();
void InitLua();
void InitSystems();
void Update();
void UpdateLua(const float&);
void Render();
void CleanUp();