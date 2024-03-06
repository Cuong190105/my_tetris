#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

SDL_Window *game_window = NULL;

SDL_Renderer *renderer = NULL;

bool init()
{
    //Initialization status flag
    bool success = true;

    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        cout << "Failed to initialize SDL" << endl;
        success = false;
    }
    else
    {
        //Create game window
        game_window = SDL_CreateWindow( "Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
        if ( game_window == NULL )
        {
            cout << "Failed to create window" << endl;
            success = false;
        }
        else 
        {
            //Create renderer
            renderer = SDL_CreateRenderer( game_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
            if ( renderer == NULL )
            {
                cout << "Failed to create renderer" << endl;
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
            }
        }

    }
    return success;
}

void close()
{
    //Destroy window
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( game_window );
    game_window = NULL;
    renderer = NULL;

    //Quit SDL
    SDL_Quit();
}

int main(int argv, char **args) {
    //Initialize SDL & Create window
    if ( !init() ) {
        cout << "Failed to initialize" << endl;
    }
    else
    {
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        while ( !quit )
        {
            //Handle events
            while ( SDL_PollEvent(&e) != 0 ) 
            {
                //Quitting game event
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }


                //Update screen
                SDL_RenderPresent( renderer );
            }
        }
    }
    close();
    return 0;
}