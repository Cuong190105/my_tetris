#ifndef logic_hpp
#define logic_hpp
#include "rendering.hpp"
using namespace std;

void gameHandler( int players, int gameMode, int mod[3], int &scene, bool &transIn );

void generateTetromino( vector<int> &Tqueue );

int adjustmentButton( int x, int y, bool disableLeft, bool disableRight );

void settingRules( bool isSolo, int gameMode, int &activeButton, bool &adjusted, int mod[4] );

void taskManager();

void menuManager( int &scene, bool &transIn, int &players, int &gameMode, int mod[4] );

void gameSettings( int &scene, int &activeButton );

void pause();

void applySettings();
#endif