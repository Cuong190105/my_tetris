#ifndef rendering_hpp
#define rendering_hpp
#include <SDL.h>
#include "Player.hpp"
#include "Texture.hpp"

//Renders the playfield
void renderBoard( const PlayBoard &pb );

//Renders the currently playing tetromino and its ghost on the field
void renderCurrentTetromino( const PlayBoard &pb, const Tetromino &tetr, int ghostRow );

/**
 * Renders preview tetrominos in Hold/Queue container
 * \param x, y Position of preview box's top left corner
 */
void renderPreviewTetromino( int x, int y, const Tetromino &tetr );

//Renders the tetromino queue
void renderTetrominoQueue( const PlayBoard &pb, const vector<Tetromino>& Tqueue );

//Renders the held tetromino
void renderHeldTetromino( const PlayBoard &pb, const Tetromino &tetr );

/**
 * Renders text
 * \param text Text to be rendered
 * \param x X-coordinate of textbox's top-left/right corner ( takes left side if text is left/center aligned, else right side )
 * \param y Y-coordinate of textbox's top-left/right corner
 * \param alignment Indicates how text should be aligned in the box
 */
void renderText( string text, int x, int y, int alignment, double scale = 1 );

void renderStatistics( const Player &player );

//Clears screen
void clearScreen();

//Renders the whole frame
void renderFrame( const Player &player, const vector<Tetromino> &Tqueue );

//Initializes SDL system, game window and renderer
bool init();

//Loads media
void loadMedia();

//Loads random background
void loadRandomBackground();

//Terminate SDL system and close window
void close();

#endif