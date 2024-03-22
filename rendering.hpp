#ifndef rendering_hpp
#define rendering_hpp
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
 * \param isBold Indicates whether the text should be bold
 * \param isRightAligned Indicates how text should be aligned in the box
 * \param scale Adjusts text size
 * \param color Adjusts text color
 */
void renderText( string text, int x, int y, bool isBold, bool isRightAligned, double scale = 1, SDL_Color color = {255, 255, 255} );

void renderStatistics( const PlayBoard &pb, const int stat[] );

//Clears screen
void clearScreen();

//Renders the whole frame
void renderFrame( const Player &player, const vector<Tetromino> &Tqueue );

//---------------------------MAIN MENU---------------------------
//Main menu buttons
const int BUTTONS = 4;
enum Button { SOLO_BUTTON, MULTI_BUTTON, SETTINGS_BUTTON, QUIT_BUTTON };
extern SDL_Rect buttonBox[BUTTONS];
const int BUTTON_X = TILE_WIDTH * 8;
const int SOLO_BUTTON_Y = TILE_WIDTH * 18;
const int BUTTON_HEIGHT = TILE_WIDTH * 3;
const int BUTTON_WIDTH = TILE_WIDTH * 16;
const int BUTTON_PADDING = TILE_WIDTH;

//Renders main menu screen
void renderMenuBackground();

void renderMainMenuButton( int mouse_x, int mouse_y, int &activeButton );


//--------------------------INIT & LOAD--------------------------
//Initializes SDL system, game window and renderer
bool init();

//Loads media
void loadMedia();

//Loads random background
void loadRandomBackground();

//Terminate SDL system and close window
void close();

#endif