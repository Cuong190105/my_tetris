#include <iostream>
#include <queue>
#include <cstdlib>
#include "logic.hpp"
#include "rendering.hpp"
#include <fstream>
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

        //Game loop
        while ( !quit )
        {
            ifstream file("data_test.txt");
            int data;
            //Initialize game & piece bag;
            PlayBoard board;
            queue<Tetrimino> TetriminoQueue;
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 10; j++) {
                    file >> data;
                    board.modifyCell(i, j, data);
                }
            }
            file.close();
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