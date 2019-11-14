
#include "SDL_Static_Helper.h"

namespace SDL_Input {

    bool fullscreen = false;
    SDL_Event windowEvent;
    SDL_Window* _window;
    float xRel = 0;
    float yRel = 0;
    const Uint8* keyboard;
	KeyState keyState;

}