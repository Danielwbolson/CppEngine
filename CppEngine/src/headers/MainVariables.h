
// Holds main function variables and include statements

#include <SDL.h>
#include "glad/glad.h"
#include "SDL_Static_Helper.h"

#include "Systems.h"
#include "MeshRendererSystem.h"
#include "ColliderSystem.h"
#include "PhysicsSystem.h"

#include "Globals.h"
#include "Camera.h"
#include "Scene.h"
#include "AssetManager.h"
#include "MemoryManager.h"

#include "Map.h"
#include "Configuration.h"

#include "lua-5.3.5/src/lua.hpp"
#include "LuaSupport.h"
#include "sol/sol.hpp"

const std::string mapFile = "map.txt";
const std::string sceneFile = "scene.txt";
std::string gameFolder = "";
std::string luaMain = "";

sol::state lua;
sol::function frameUpdate;

int screenWidth = 1920;
int screenHeight = 1080;
int engineComponentsSize = 3;

float luaTimeSpeed = 1;
float startTime, printTime;
bool quit = false;

std::vector<Systems*> systems;
Map* map;

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