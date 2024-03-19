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

/**
 * Renders text
 * \param text Text to be rendered
 * \param x X-coordinate of textbox's top-left/right corner ( takes left side if text is left/center aligned, else right side )
 * \param y Y-coordinate of textbox's top-left/right corner
 * \param alignment Indicates how text should be aligned in the box
 */
void renderText( string text, int x, int y, int alignment, double scale = 1 );

void renderStatistics( const PlayBoard &pb );

//Clears screen
void clearScreen();

//Renders the whole frame
void renderFrame( const PlayBoard &pb, const Tetrimino &tetr, const vector<Tetrimino> &Tqueue, const Tetrimino &hold );

//Initializes SDL system, game window and renderer
bool init();

//Loads media
void loadMedia();

//Loads random background
void loadRandomBackground();

//Terminate SDL system and close window
void close();

#endif