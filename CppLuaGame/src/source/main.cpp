
// Holds variables, functions and include statements necessary for the main function
#include "MainVariables.h"

/* 
 * Main game loop for engine:
 * - Takes input
 * - Updates all components and systems
 * - Renders all components and systems
 */
int main(int argc, char* argv[]) {

    // Initalize, but check for initlization errors
    if (Init() == -1)
        return -1;

    // Game loop
    while (!quit) {
        SDL_Input::SDLEvents(&quit);
        Update();
        Render();
    }

    // SDL and pointer cleanup
    CleanUp();

    return 0;
}



/* * * * * * * * * *  HELPER FUNCTIONS  * * * * * * * * * */



int Init() {

    // Initilize SDL
    int status = SDL_Input::SDLInit(&window, &context, screenWidth, screenHeight);
    if (status == -1) {
        return -1;
    }

    // Initalize our camera
    mainCamera = new Camera();

    // Get input map and scene files
    scene = Scene();
    std::string sceneFile = "LightScene.scn";
    scene = SceneParse(scene, sceneFile);

    map = Map();
    std::string mapFile = "Map1.map";
    map = MapParse(map, mapFile, &scene);

    // Initialize our systems
    InitSystems();

    // Start our time calculations for real-time interaction
    startTime = SDL_GetTicks() / 1000.0f;
    printTime = startTime;

    return 0;
}

void InitSystems() {
    systems = std::vector<Systems*>();

    // Add in new engine components
    systems.push_back(new MeshRendererSystem(screenWidth, screenHeight));
    systems.push_back(new ColliderSystem());
    systems.push_back(new PhysicsSystem());

    for (int i = 0; i < (int)systems.size(); i++) {
        systems[i]->Setup(scene.instances, scene.lights);
    }
}

void Update() {
    // Get frame time
    float dt = (SDL_GetTicks() / 1000.0f) - startTime;
    startTime = SDL_GetTicks() / 1000.0f;

    // Printing out fps
    if (startTime - printTime > 2) {
        fprintf(stderr, "fps: %f\n", 1.0f / dt);
        printTime = startTime;
    }

    //systems.CollisionChecks(dt)
    for (int i = 0; i < engineComponentsSize; i++) {
        systems[i]->Update(dt);
    }

    scene.CollisionChecks(dt);
    scene.Update(dt);
    mainCamera->Update(dt);
}

void Render() {
    for (int i = 0; i < engineComponentsSize; i++) {
        systems[i]->Render();
    }

    SDL_GL_SwapWindow(window); //Double buffering
}

void CleanUp() {
    SDL_GL_DeleteContext(context);
    SDL_Quit();

    for (int i = 0; i < engineComponentsSize; i++) {
        delete systems[i];
    }
    delete mainCamera;

    systems.clear();
}
