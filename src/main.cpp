#include <SDL.h>
#include <iostream>

// The signature MUST be (int, char**) for SDL compatibility on Windows
int main(int argc, char* argv[]) {
    // 1. Tell SDL we are handling main ourselves
    SDL_SetMainReady();

    // 2. Initialize SDL (if you are using it yet)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        // Handle error
    }

    std::cout << "FIMS Node Starting..." << std::endl;
    
    // ... your logic ...

    return 0;
}
