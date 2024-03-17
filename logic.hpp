#ifndef logic_hpp
#define logic_hpp
#include "Tetrimino.hpp"
#include <SDL.h>
using namespace std;

void generateTetrimino( vector<Tetrimino> &Tqueue );

void pullNewTetrimino( vector<Tetrimino> &Tqueue, Tetrimino &tetr );

void handlingKeyPress( SDL_Event &e, PlayBoard &pb, Tetrimino &tetr, Tetrimino &held, bool &holdable );
#endif