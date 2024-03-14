#ifndef logic_hpp
#define logic_hpp
#include "Tetrimino.hpp"
using namespace std;

void updateBoard( PlayBoard pb, Tetrimino tetr );

bool checkCollision( PlayBoard pb, Tetrimino tetr, int rowAdjustment = 0, int colAdjustment = 0 );

#endif