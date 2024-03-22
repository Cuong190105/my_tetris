#include <iostream>
#include <vector>
#include <cstdlib>
#include "logic.hpp"
using namespace std;

int main(int argv, char **args) {
    //Initialize SDL & Create window
    if ( !init() ) {
        cout << "Failed to initialize" << endl;
    }
    else
    {
        srand(time(NULL));
        loadMedia();
        loadRandomBackground();

        const int SCREEN_FPS = 60;
        const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
        Uint32 frameMark, frameTicks;

        int scene = MAIN_MENU;
        //Game loop
        while ( scene != QUIT )
        {
            frameMark = SDL_GetTicks();
            clearScreen();
            gameManager( scene );
            SDL_RenderPresent( renderer );
            frameTicks = SDL_GetTicks() - frameMark;
            if ( frameTicks < SCREEN_TICKS_PER_FRAME )
            {
                SDL_Delay( SCREEN_TICKS_PER_FRAME - frameTicks );
            }
            // while ( SDL_PollEvent(&event) )
            // {
            //     if ( event.type == SDL_QUIT ) scene = QUIT;
            // }
        }
    }
    close();
    return 0;
}