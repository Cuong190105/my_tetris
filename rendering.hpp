#ifndef rendering_hpp
#define rendering_hpp
#include "Player.hpp"
#include <vector>
#include <SDL_image.h>

enum HorizontalAlignment { LEFT, CENTER, RIGHT };
enum VerticalAlignment { TOP, MIDDLE, BOTTOM };
/**
 * Renders text
 * \param text Text to be rendered
 * \param x Textbox's position X, depends on how the text is aligned: Left X if left aligned, Center X if center aligned, Right X if right aligned)
 * \param y Textbox's position Y, depends on how the text is aligned: Top Y if top aligned, Middle Y if middle aligned, Bottom X if right aligned)
 * \param isBold Indicates whether the text should be bold
 * \param Halign Indicates text's horizontal alignment
 * \param Valign Indicates text's vertical alignment
 * \param scale Adjusts text size
 * \param color Adjusts text color
 */
void renderText( string text, int x, int y, bool isBold, int Halign, int Valign, double scale = 1, SDL_Color color = {255, 255, 255} );

void renderStatistics( const Player& player, Uint32 startMark, int countDownMark = 0, int lineTarget = 0 );

bool displayCountdown( int x, int y, int w, int h, Uint32 startMark);
//Clears screen
void clearScreen();

bool displayCountdown( int x, int y, int w, int h, Uint32 startMark );

//---------------------------MAIN MENU---------------------------
//Main menu buttons
const int MAIN_MENU_BUTTONS = 4;
extern SDL_Rect buttonBox[MAIN_MENU_BUTTONS];
const int MAIN_MENU_BUTTON_X = TILE_WIDTH * 8;
const int MAIN_MENU_FIRST_BUTTON_Y = TILE_WIDTH * 18;
const int MAIN_MENU_BUTTON_HEIGHT = TILE_WIDTH * 3;
const int MAIN_MENU_BUTTON_WIDTH = TILE_WIDTH * 16;
const int MAIN_MENU_BUTTON_PADDING = TILE_WIDTH;

//Renders main menu screen
void renderMenuBackground( bool stop = false );

void renderMainMenuButton( int mouse_x, int mouse_y, int &activeButtons );

/**
 * Renders individual tetromino in the targeted texture, then the texture will float on screen
 * \param x, y Position of tetromino's top left corner
 */
void renderMenuTetromino( int _x, int _y, const Tetromino &tetr );

void renderFloatingTetromino( vector<Tetromino> &floating );

void renderGameTitle( Texture &title);

bool handleStartButton( int mouse_x, int mouse_y, int center_x, int middle_y );

bool handleBackButton( int mouse_x, int mouse_y );

void renderTransition( bool &transIn );

//---------------------------SOLO MENU---------------------------
const int SOLO_MENU_BUTTONS = 5;
extern SDL_Rect soloMenuButtonBox[SOLO_MENU_BUTTONS];
const int SOLO_MENU_BUTTON_X = TILE_WIDTH * 8;
const int SOLO_MENU_FIRST_BUTTON_Y = TILE_WIDTH * 10;
const int SOLO_MENU_BUTTON_HEIGHT = TILE_WIDTH * 4;
const int SOLO_MENU_BUTTON_WIDTH = TILE_WIDTH * 20;
const int SOLO_BUTTON_PADDING = TILE_WIDTH / 5;
extern const string soloGameModeName[];
void renderSoloMenu( int mouse_x, int mouse_y, int &activeButton );

const int LEFT_ADJUSTMENTBUTTON_X = -TILE_WIDTH * 6;;
const int RIGHT_ADJUSTMENTBUTTON_X = TILE_WIDTH * 5;
void renderAdjustmentButton( int x, int y, bool disableLeft, bool disableRight );

void loadRandomBackground();

void loadMenuElements();

void renderResultScreen( const Player &player, Uint32 startMark, string time, bool fadeOut = false );

int renderRetryScreen( bool &retryLoop, int &scene );

#endif