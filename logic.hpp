#ifndef logic_hpp
#define logic_hpp
#include "rendering.hpp"
using namespace std;

//===================================INGAME MANIPULATION================================

//Handle pause events.
void handlePauseMenu( int &activeButton, int &mouse_x, int &mouse_y );

//Handles ingame events. Displays play field
void gameHandler( int players, int gameMode, int mod[3], int &scene, bool &transIn );

//Creates tetrominos & push it into the queue if needed
void generateTetromino( vector<Tetromino> &Tqueue, bool forceAdd = false );

//===================================HIGH SCORE MANIPULATION================================

const string HIGHSCORE_DIR = "src/hiscore.txt";

//Updates high score
void updateHighScore( int mode, int score, int line, int time );

//Loads high score
void loadHighScore();

//Saves high score
void saveHighScore();


//==================================SINGLEPLAYER MANIPULATION================================

//Sets & displays rules for singleplayer modes.
void settingRules( int gameMode, int &activeButton, bool &adjusted, int mod[4] );

//===================================MULTIPLAYER MANIPULATION================================
void multiplayerManager( int scene, int &changeScene, int mouse_x, int mouse_y, int &activeButton, bool isClicked, SDL_Keycode key, string text = "" );

void processOpponentsControl( vector<Tetromino> &Tqueue, vector<int> &queuePosition, vector<Player> &player, vector<int>& recentEliminated, int &survivors );
//=======================================SETTINGS============================================

//Manages game settings
void gameSettings( int &scene, int &activeButton, int &adjusted, int mouse_x, int mouse_y );

//Handles key bindings events: Display prompts, get key inputs, change key bindings
bool handleKeybindButton( string content, int mouse_x, int mouse_y, int x, int y, int w, int h, SDL_Color bg = { 0, 0, 0 }, SDL_Color txtColor = { 255, 255, 255 });

//Applies loaded settings
void applySettings( int type = -1 );

//Handles adjustment buttons: Display & function
int adjustmentButton( int x, int y, bool disableLeft, bool disableRight );

//====================================MENU MANAGER===========================================
//Handles the whole application.
void taskManager();

//Handles menu scenes & events
void menuManager( int &scene, bool &transIn, int &players, int &gameMode, int mod[4] );
#endif