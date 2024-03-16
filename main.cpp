#include <iostream>
#include <vector>
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
        loadMedia();
        //Main loop flag
        bool quit = false;
        srand(time(NULL));
        //Event handler
        SDL_Event e;

        //Initialize game & piece bag;
        PlayBoard board;
        vector<Tetrimino> TetriminoQueue;
        Tetrimino currentTetr = generateTetrimino();
        board.modifyCell( 10, 5, 3 );
        //Game loop
        while ( !quit )
        {
            // while ( TetriminoQueue.size() < 5 ) {
            //     TetriminoQueue.push_back( generateTetrimino() );
            // }
            //Handle events
            while ( SDL_PollEvent(&e) != 0 ) 
            {
                //Quitting game event
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }

                handlingKeyPress( e, board, currentTetr );
            }
            
            SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
            SDL_RenderClear( renderer );

            renderBoard( board );
            renderCurrentTetrimino( board, currentTetr );

            //Update screen
            SDL_RenderPresent( renderer );
        }
    }
    close();
    return 0;
}