#ifndef rendering_hpp
#define rendering_hpp
#include <SDL.h>
#include "Tetrimino.hpp"
#include "Texture.hpp"

//Renders the playfield
void renderBoard( const PlayBoard &pb );

//Renders the currently playing tetrimino and its ghost on the field
void renderCurrentTetrimino( const PlayBoard &pb, const Tetrimino &tetr );

/**
 * Renders preview tetriminos in Hold/Queue container
 * \param x, y Position of preview box's top left corner
 */
void renderPreviewTetrimino( int x, int y, const Tetrimino &tetr );

//Renders the tetrimino queue
void renderTetriminoQueue( const PlayBoard &pb, const vector<Tetrimino>& Tqueue );

//Renders the held tetrimino
void renderHeldTetrimino( const PlayBoard &pb, const Tetrimino &tetr );

//Clears screen
void clearScreen();

//Renders the whole frame
void renderFrame( const PlayBoard &pb, const Tetrimino &tetr, const vector<Tetrimino> &Tqueue );

//Initializes SDL system, game window and renderer
bool init();

//Loads media
void loadMedia();

//Terminate SDL system and close window
void close();

#endif