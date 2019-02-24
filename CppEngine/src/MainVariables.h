
// Holds main function variables and include statements

#include "../glad/glad.h"  //Include order can matter here
#include "../glad/glad.c"

#include <SDL.h>
#include <SDL_opengl.h>

#include "Parse.h"
#include "Scene.h"
#include "SDL_Static_Helper.h"
#include "Camera.h"

#include "Systems.h"
#include "MeshRendererSystem.h"
#include "ColliderSystem.h"
#include "PhysicsSystem.h"

#define GLM_FORCE_RADIANS //ensure we are using radians

int screenWidth = 1920;
int screenHeight = 1080;
int engineComponentsSize = 3;

float startTime, printTime;
bool quit = false;

// Our Systems and camera
std::vector<Systems*> systems;
Camera* mainCamera;

// SDL objects
SDL_Window* window;
SDL_GLContext context;

// Scene and map objects
Scene scene;
Map map;

// Helper functions
int Init();
void Update();
void Render();
void InitSystems();
void CleanUp();