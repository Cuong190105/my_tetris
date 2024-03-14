#ifndef rendering_hpp
#define rendering_hpp
#include <SDL.h>
// #include <SDL_image.h>
#include "Tetrimino.hpp"

extern SDL_Renderer *renderer;

//Render the playfield
void renderBoard( PlayBoard pb );

//Initialize SDL system, game window and renderer
bool init();

//Terminate SDL system and close window
void close();

#endif