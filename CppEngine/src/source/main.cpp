
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
    int status = SDL_Input::SDLInit(&window, &context, windowWidth, windowHeight);
    if (status == -1) {
        return -1;
    }

	performanceStream.open("../perfLog.csv");
	performanceStream << "FPS, Triangles, Culling, Shadows, Deferred To Texture, Deferred Lighting, Forward Lighting, Post Processing" << std::endl;

	/* mainCamera is defined in assetManager */
	assetManager = new AssetManager();
	memoryManager = new MemoryManager();

	// Initialize lua and other input files
	InitLua();

    // Initialize our systems
    InitSystems();

    // Start our time calculations for real-time interaction
    startTime = SDL_GetTicks() / 1000.0f;
    printTime = startTime;

    return 0;
}

void InitLua() {

	luaSetup(lua);

	mainScene = assetManager->LoadScene(gameFolder + sceneFile);
	assetManager->LoadGameObjects(gameFolder + gameObjectsFile, mainScene);

	auto script_from_file_result = lua.safe_script_file(VK_ROOT_DIR + luaMain, sol::script_pass_on_error);
	if (!script_from_file_result.valid()) {
		sol::error err = script_from_file_result;
		std::cerr << "The code from the file has failed to run!\n" << err.what() << "\nPanicking and exiting..." << std::endl;
		exit(1);
	}

	frameUpdate = lua["frameUpdate"];
	
	lua.new_usertype<SDL_Input::KeyState>("key_state",
		"one", &SDL_Input::KeyState::one,
		"two", &SDL_Input::KeyState::two,
		"three", &SDL_Input::KeyState::three,
		"four", &SDL_Input::KeyState::four,
		"five", &SDL_Input::KeyState::five,
		"six", &SDL_Input::KeyState::six
	);
	keyInput = lua["keyInput"];
	lua["keys"] = &SDL_Input::keyState;
}

void InitSystems() {
    systems = std::vector<Systems*>();

    // Add in new engine components
    systems.push_back(new RendererSystem(windowWidth, windowHeight));
    systems.push_back(new ColliderSystem());
    systems.push_back(new PhysicsSystem());

    for (int i = 0; i < (int)systems.size(); i++) {
        systems[i]->Setup();
    }
}

void Update() {
    // Get frame time
    float dt = (SDL_GetTicks() / 1000.0f) - startTime;
	float luaDt = luaTimeSpeed * dt;
    startTime = SDL_GetTicks() / 1000.0f;

    // Printing out fps
    if (startTime - printTime > 0.5) {
		performanceStream << 1.0f / dt << ",";
		performanceStream << ((RendererSystem*)systems[0])->totalTriangles << ",";
		performanceStream << ((RendererSystem*)systems[0])->cullTime << ",";
		performanceStream << ((RendererSystem*)systems[0])->shadowTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)systems[0])->deferredToTexTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)systems[0])->deferredLightsTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)systems[0])->transparentTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)systems[0])->postFXXTime / 1000.0f << "," << std::endl;
        printTime = startTime;
    }

    //systems.CollisionChecks(dt)
    for (int i = 0; i < engineComponentsSize; i++) {
        systems[i]->Update(dt);
    }

    mainScene->CollisionChecks(dt);
	mainScene->Update(dt);
    mainCamera->Update(dt);

	UpdateLua(luaDt);
}

void UpdateLua(const float& luaDt) {
	frameUpdate(luaDt);
	keyInput(lua["keys"]);
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
	delete mainScene;
	delete assetManager;
	delete memoryManager;

	performanceStream.close();
}
