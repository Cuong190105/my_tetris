#ifndef rendering_hpp
#define rendering_hpp
#include <SDL.h>
#include "Tetrimino.hpp"
#include "Texture.hpp"

//Render the playfield
void renderBoard( PlayBoard pb );

//Render the currently playing tetrimino on the field
void renderCurrentTetrimino( PlayBoard pb, Tetrimino tetr );

//Render the tetrimino queue
void renderTetriminoQueue( vector<Tetrimino> Tqueue );

//Render the held tetrimino
void renderHeldTetrimino( Tetrimino tetr );

//Initialize SDL system, game window and renderer
bool init();

//Loads media
void loadMedia();

//Terminate SDL system and close window
void close();

#endif