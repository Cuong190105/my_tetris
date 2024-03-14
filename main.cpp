#include <iostream>
#include <queue>
#include <cstdlib>
#include "logic.hpp"
#include "rendering.hpp"
using namespace std;

int main(int argv, char **args) {
    //Initialize SDL & Create window
    if ( !init() ) {
        cout << "Failed to initialize" << endl;
    }
    else
    {
        //Main loop flag
        bool quit = false;
        srand(time(NULL));
        //Event handler
        SDL_Event e;

        //Game loop
        while ( !quit )
        {

            //Initialize game & piece bag;
            PlayBoard board;
            queue<Tetrimino> TetriminoQueue;

            //Handle events
            while ( SDL_PollEvent(&e) != 0 ) 
            {
                //Quitting game event
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                // while ( TetriminoQueue.size() < 5 ) {
                //     TetriminoQueue.push( generateNewTetrimino() );
                // }


                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
                SDL_RenderClear( renderer );

                renderBoard( board );
                

                //Update screen
                SDL_RenderPresent( renderer );
            }
        }
    }
    close();
    return 0;
}