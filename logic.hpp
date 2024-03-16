#ifndef logic_hpp
#define logic_hpp
#include "Tetrimino.hpp"
#include <SDL.h>
using namespace std;

void updateBoard( PlayBoard pb, Tetrimino tetr );

Tetrimino generateTetrimino();

void handlingKeyPress( SDL_Event &e, const PlayBoard &pb, Tetrimino &tetr );
#endif