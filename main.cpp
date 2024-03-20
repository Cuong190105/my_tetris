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
        //Event handler
        // SDL_Event e;

        //Main loop flag
        bool quit = false;
        //Game loop
        while ( !quit )
        {
            ingameProgress();
            quit = true;
        }
    }
    close();
    return 0;
}