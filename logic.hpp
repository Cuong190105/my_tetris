#ifndef logic_hpp
#define logic_hpp
#include "rendering.hpp"
using namespace std;

enum gameScene {MAIN_MENU, SOLO_MENU, MULTI_MENU, SETTINGS, QUIT};
extern SDL_Event event;

void generateTetromino( vector<int> &Tqueue );

void handlingKeyPress( SDL_Event &e, Player &player );

void ingameProgress();

void gameManager( int &scene );

void handlingMouseClick( int &scene, int activeButton );
#endif