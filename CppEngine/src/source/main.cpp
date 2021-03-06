
// Holds variables, functions and include statements necessary for the main function
#include "MainVariables.h"
#include "GlobalMacros.h"

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
#if RAY_TRACING_ENABLED
	performanceStream << "FPS, Ray Trace, Post Processing" << std::endl;
#else
	performanceStream << "FPS, Triangles, Culling, Depth Pre Pass, Shadows, Deferred To Texture, Tiled Deferred (Point), Deferred Lighting (Directional), Forward Lighting (Transparent), Post Processing" << std::endl;
#endif
	/* mainCamera is defined in assetManager */
	MemoryManager::Init();
	AssetManager::Init();

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

	mainScene = AssetManager::LoadScene(gameFolder + sceneFile);
	AssetManager::LoadGameObjects(gameFolder + gameObjectsFile, mainScene);

	auto script_from_file_result = lua.safe_script_file(VK_ROOT_DIR + luaMain, sol::script_pass_on_error);
	if (!script_from_file_result.valid()) {
		sol::error err = script_from_file_result;
		std::cerr << "The code from the file has failed to run!\n" << err.what() << "\nPanicking and exiting..." << std::endl;
		exit(1);
	}

	AssetManager::PostLoadScene();

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
    systems = MemoryManager::Allocate<std::vector<Systems*, MemoryAllocator<Systems*> > >();

    // Add in new engine components
	if (RAY_TRACING_ENABLED) {
		systems->push_back(MemoryManager::Allocate<RayTracingSystem>());
	}
	else {
		systems->push_back(MemoryManager::Allocate<RendererSystem>());
	}

    systems->push_back(MemoryManager::Allocate<ColliderSystem>());
    systems->push_back(MemoryManager::Allocate<PhysicsSystem>());

    for (int i = 0; i < (int)systems->size(); i++) {
        (*systems)[i]->Setup();
    }
}

void Update() {

	static float lastFPStime = -10000.0f;
	// Get frame time
	float dt = (SDL_GetTicks() / 1000.0f) - startTime;
	float luaDt = luaTimeSpeed * dt;
	startTime = SDL_GetTicks() / 1000.0f;

	if (startTime - lastFPStime > 1.0f)
	{
		int32_t fps = (int32_t)(1.0f / dt);
		std::string title = "FPS: " + std::to_string(fps);
		SDL_SetWindowTitle(window, title.c_str());
		lastFPStime = startTime;
	}

    // Printing out fps
#if RAY_TRACING_ENABLED
	if (startTime - printTime > 0.5) {
		performanceStream << 1.0f / dt << ",";
		performanceStream << ((RayTracingSystem*)(*systems)[0])->rayTraceTime / 1000.0f << ",";
		performanceStream << ((RayTracingSystem*)(*systems)[0])->postFXTime / 1000.0f << "," << std::endl;
		printTime = startTime;
	}
#else
    if (startTime - printTime > 0.5) {
		performanceStream << 1.0f / dt << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->totalTriangles << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->cullTime << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->depthPrePassTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->shadowTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->deferredToTexTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->tileComputeTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->deferredLightsTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->transparentTime / 1000.0f << ",";
		performanceStream << ((RendererSystem*)(*systems)[0])->postFXXTime / 1000.0f << "," << std::endl;
        printTime = startTime;
    }
#endif
    //systems.CollisionChecks(dt)
    for (int i = 0; i < engineComponentsSize; i++) {
        (*systems)[i]->Update(dt);
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
        (*systems)[i]->Render();
    }

    SDL_GL_SwapWindow(window); //Double buffering
}

void CleanUp() {
    SDL_GL_DeleteContext(context);
    SDL_Quit();

    for (int i = 0; i < engineComponentsSize; i++) {
		MemoryManager::Free((*systems)[i]);
    }
    MemoryManager::Free(mainCamera);
	MemoryManager::Free(mainScene);

	AssetManager::CleanUp();
	MemoryManager::CleanUp();

	performanceStream.close();
}
