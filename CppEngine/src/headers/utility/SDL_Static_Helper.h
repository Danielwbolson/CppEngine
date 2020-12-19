
#ifndef SDL_STATIC_HELPER_H_
#define SDL_STATIC_HELPER_H_

#include <SDL.h>

#define GLM_FORCE_RADIANS
#include "glad/glad.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace SDL_Input {

	struct KeyState {
		bool one, two, three, four, five, six;
	};

    extern bool fullscreen;
    extern SDL_Event windowEvent;
    extern SDL_Window* _window;
    extern float xRel;
    extern float yRel;
    extern const Uint8* keyboard;
	extern KeyState keyState;

    static int SDLInit(SDL_Window** window, SDL_GLContext* context, int screenWidth, int screenHeight) {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);  //Initialize Graphics (for OpenGL)
    
        //Print the version of SDL we are using
        SDL_version comp; SDL_version linked;
        SDL_VERSION(&comp); SDL_GetVersion(&linked);
        printf("\nCompiled against SDL version %d.%d.%d\n", comp.major, comp.minor, comp.patch);
        printf("Linked SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);
    
        //Ask SDL to get a recent version of OpenGL (4.1 or greater)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
        //Create a window (offsetx, offsety, width, height, flags)
        *window = SDL_CreateWindow("My OpenGL Program", 100, 100,
                screenWidth, screenHeight, SDL_WINDOW_OPENGL);
        if (!*window) {
            printf("Could not create window: %s\n", SDL_GetError());
            return EXIT_FAILURE; //Exit as SDL failed
        }
        float aspect = screenWidth / (float)screenHeight; //aspect ratio needs update on resize
        *context = SDL_GL_CreateContext(*window); //Bind OpenGL to the window
    
        if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
            printf("OpenGL loaded\n");
            printf("Vendor:   %s\n", glGetString(GL_VENDOR));
            printf("Renderer: %s\n", glGetString(GL_RENDERER));
            printf("Version:  %s\n", glGetString(GL_VERSION));
        }
        else {
            printf("ERROR: Failed to initialize OpenGL context.\n");
            return -1;
        }
        _window = *window;

		SDL_ShowCursor(SDL_DISABLE);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_GL_SetSwapInterval(0);
        keyboard = SDL_GetKeyboardState(NULL);

        return 0;
    }

    static void SDLEvents(bool* quit) {
        SDL_PumpEvents();
        xRel = 0;
        yRel = 0;

		keyState.one = keyboard[SDL_SCANCODE_1];
		keyState.two = keyboard[SDL_SCANCODE_2];
		keyState.three = keyboard[SDL_SCANCODE_3];
		keyState.four = keyboard[SDL_SCANCODE_4];
		keyState.five = keyboard[SDL_SCANCODE_5];
		keyState.six = keyboard[SDL_SCANCODE_6];

        while (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) *quit = true; //Exit Game Loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
                *quit = true; //Exit Game Loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) {
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
            }
            if (windowEvent.type == SDL_MOUSEMOTION) {
                xRel = windowEvent.motion.xrel * 0.01f;
                yRel = windowEvent.motion.yrel * 0.01f;
            }
        }
    }
}

#endif