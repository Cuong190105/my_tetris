#ifndef rendering_hpp
#define rendering_hpp
#include "Player.hpp"
#include <vector>
#include <SDL_image.h>

enum HorizontalAlignment { LEFT, CENTER, RIGHT };
enum VerticalAlignment { TOP, MIDDLE, BOTTOM };

extern SDL_Window *game_window;
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

void renderParagraph( string text, int width, int x_origin, int y_origin, bool isBold = false, SDL_Color color = {255, 255, 255} );

void renderStatistics( const Player& player, Uint32 startMark, int countDownMark = 0, int lineTarget = 0 );

bool displayCountdown( int x, int y, int w, int h, Uint32 startMark);
//Clears screen
void clearScreen();

bool displayCountdown( int x, int y, int w, int h, Uint32 startMark );

//---------------------------MAIN MENU---------------------------
//Main menu buttons
const int MAIN_MENU_BUTTONS = 4;
extern SDL_Rect buttonBox[MAIN_MENU_BUTTONS];
extern int MAIN_MENU_BUTTON_X;
extern int MAIN_MENU_FIRST_BUTTON_Y;
extern int MAIN_MENU_BUTTON_HEIGHT;
extern int MAIN_MENU_BUTTON_WIDTH;
extern int MAIN_MENU_BUTTON_PADDING;

//Renders main menu screen
void renderMenuBackground( bool stop = false );

void renderMainMenuButton( int mouse_x, int mouse_y, int &activeButtons );

/**
 * Renders individual tetromino in the targeted texture, then the texture will float on screen
 * \param x, y Position of tetromino's top left corner
 */
void renderMenuTetromino( int _x, int _y, const Tetromino &tetr );

//Renders tetrominos floating on the background
//\param floating A vector containing the floating tetrominos.
void renderFloatingTetromino( vector<Tetromino> &floating );

//Renders game title on the main menu
void renderGameTitle( Texture &title );

/**
 * Renders & handles start button events.
 * \param mouse_x X position of the mouse
 * \param mouse_y Y position of the mouse
 * \param center_x X position of the center point of the start button
 * \param middle_y Y position of the center point of the start button
 * */
bool handleStartButton( int mouse_x, int mouse_y, int center_x, int middle_y );

/**
 * Render and handles back button event.
 * \param mouse_x X position of the mouse
 * \param mouse_y Y position of the mouse
*/
bool handleBackButton( int mouse_x, int mouse_y );

/**
 * Renders fading transition between MENU and INGAME scene.
 * \param transIn Decide whether the transition should be fade in or out.
*/
void renderTransition( bool &transIn );

//---------------------------SOLO MENU---------------------------
const int SOLO_MENU_BUTTONS = 5;
extern SDL_Rect menuButtonBox[SOLO_MENU_BUTTONS];
extern int GAMEMODE_MENU_BUTTON_X;
extern int GAMEMODE_MENU_FIRST_BUTTON_Y;
extern int GAMEMODE_MENU_BUTTON_HEIGHT;
extern int GAMEMODE_MENU_BUTTON_WIDTH;
extern int GAMEMODE_BUTTON_PADDING;
extern const string gameModeName[];
void renderSoloMenu( int mouse_x, int mouse_y, int &activeButton );

//------------------------MULTIPLAYER MENU-----------------------
extern const string multiGameModeName[];
const int MULTI_MENU_BUTTONS = 3;
void renderMultiMenu( int mouse_x, int mouse_y, int &activeButton, bool isClicked );

bool renderTextInputBox( string title, string button, string &inputString, int mouse_x, int mouse_y, bool isClicked, SDL_Keycode key, string text );

bool renderMatchSettings( int mouse_x, int mouse_y, bool isClicked, SDL_Keycode key, string text );

void renderJoinServer( int mouse_x, int mouse_y, int &activeButton, int &selected, int currPage, bool isClicked, vector<string> address, vector<string> serverName );

extern bool isHost;
void renderLobby( int mouseX, int mouseY, int &activeButton );



extern int LEFT_ADJUSTMENTBUTTON_X;
extern int RIGHT_ADJUSTMENTBUTTON_X;

/**
 * Renders the adjustment buttons (the left and right pointed triangle buttons used for adjusting game preferences or settings).
 * \param x X position of the center point of the content that need adjusting.
 * \param y Y position of the center point of the content that need adjusting.
 * \param disableLeft Indicates that the left button should be disabled (greyed out & non-clickable).
 * \param disableRight Indicates that the right button should be disabled (greyed out & non-clickable).
*/
void renderAdjustmentButton( int x, int y, bool disableLeft, bool disableRight );

//Loads a random ingame background
void loadRandomBackground();

//Loads menu elements: Button Size, Background, Game Title,...
void loadMenuElements();

//Renders the result screen
void renderResultScreen( const Player &player, Uint32 startMark, string time );

int renderRetryScreen( bool &retryLoop, int &scene );

void changeDimensions();

void renderKeybindButton( string content, int x, int y, int w, int h, SDL_Color bg, SDL_Color txtColor );

#endif