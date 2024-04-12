#ifndef logic_hpp
#define logic_hpp
#include "rendering.hpp"
using namespace std;

void handlePauseMenu( int &activeButton, int &mouse_x, int &mouse_y );

void gameHandler( int players, int gameMode, int mod[3], int &scene, bool &transIn );

void generateTetromino( vector<int> &Tqueue );

int adjustmentButton( int x, int y, bool disableLeft, bool disableRight );

void settingRules( bool isSolo, int gameMode, int &activeButton, bool &adjusted, int mod[4] );

void taskManager();

void menuManager( int &scene, bool &transIn, int &players, int &gameMode, int mod[4] );

void gameSettings( int &scene, int &activeButton, int &adjusted, int mouse_x, int mouse_y );

bool handleKeybindButton( string content, int mouse_x, int mouse_y, int x, int y, int w, int h, SDL_Color bg = { 0, 0, 0 }, SDL_Color txtColor = { 255, 255, 255 });

//Applies loaded settings
void applySettings( int type = -1 );
#endif