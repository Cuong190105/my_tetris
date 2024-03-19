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
        srand(time(NULL));
        loadMedia();
        loadRandomBackground();
        SDL_Event e;

        //Main loop flag
        bool quit = false;
        bool gameOver = false;
        //Event handler

        //Initialize game & piece bag;
        PlayBoard board;
        vector<Tetrimino> Tqueue;
        Tetrimino currentTetr, hold;
        bool holdable = true;


        //Game loop
        while ( !quit )
        {
            clearScreen();

            if ( !gameOver && currentTetr.getType() == 0 )
            {
                pullNewTetrimino( Tqueue, currentTetr );
                if ( currentTetr.checkCollision( board ) ) {
                    gameOver = true;
                }
            }
            

            //Handle events
            while ( SDL_PollEvent(&e) != 0 ) 
            {
                //Quitting game event
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                if ( !gameOver ) handlingKeyPress( e, board, currentTetr, hold, holdable );
            }
            renderFrame( board, currentTetr, Tqueue, hold );
        }
    }
    close();
    return 0;
}