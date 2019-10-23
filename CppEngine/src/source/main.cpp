
// Holds variables, functions and include statements necessary for the main function
#include "MainVariables.h"

/* 
 * Main game loop for engine:
 * - Takes input
 * - Updates all components and systems
 * - Renders all components and systems
 * - Communicates between lua game and engine
 */
int main(int argc, char* argv[]) {

	if (argc == 2) {
		gameFolder = std::string(argv[1]);
		luaMain = gameFolder + "main.lua";
	} else {
		fprintf(stderr, "\nUSAGE: ./CppEngine games/gameFolder/ \n");
		exit(1);
	}

    // Initalize, but check for initlization errors
	if (Init() == -1) {
		fprintf(stderr, "SDL failed to init");
		exit(1);
	}

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

	/* mainCamera is defined in assetManager */
	assetManager = new AssetManager();
	memoryManager = new MemoryManager();

    // Initialize our systems
    InitSystems();

    // Start our time calculations for real-time interaction
    startTime = SDL_GetTicks() / 1000.0f;
    printTime = startTime;

    return 0;
}

void InitLua() {

	luaState = luaL_newstate();
	luaSetup(luaState);

	mainScene = assetManager->LoadScene(sceneFile);
	map = assetManager->LoadMap(mapFile, mainScene);

	int luaErr;
	luaErr = luaL_loadfile(L, luaFile.c_str());
	if (!luaErr)
		luaErr = lua_pcall(L, 0, LUA_MULTRET, 0);

}

void InitSystems() {
    systems = std::vector<Systems*>();

    // Add in new engine components
    systems.push_back(new MeshRendererSystem(screenWidth, screenHeight));
    systems.push_back(new ColliderSystem());
    systems.push_back(new PhysicsSystem());

    for (int i = 0; i < (int)systems.size(); i++) {
        systems[i]->Setup();
    }
}

void Update() {
    // Get frame time
    float dt = (SDL_GetTicks() / 1000.0f) - startTime;
    startTime = SDL_GetTicks() / 1000.0f;

    // Printing out fps
    if (startTime - printTime > 2) {
        fprintf(stderr, "fps: %f\n", 1.0f / dt);
		fprintf(stderr, "triangles drawn: %d\n", ((MeshRendererSystem*)systems[0])->totalTriangles);
        printTime = startTime;
    }

    //systems.CollisionChecks(dt)
    for (int i = 0; i < engineComponentsSize; i++) {
        systems[i]->Update(dt);
    }

    mainScene->CollisionChecks(dt);
	mainScene->Update(dt);
    mainCamera->Update(dt);

	UpdateLua();
}

void UpdateLua() {

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

	lua_close(luaState);

    for (int i = 0; i < engineComponentsSize; i++) {
        delete systems[i];
    }
    delete mainCamera;
	delete mainScene;
	delete assetManager;
	delete memoryManager;
}
