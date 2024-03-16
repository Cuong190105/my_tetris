#include <algorithm>
#include <iostream>
#include <SDL.h>
#include <cstdlib>
#include "logic.hpp"
using namespace std;

void updateBoard( PlayBoard pb, Tetrimino tetr )
{
    for ( int i = 0; i < tetr.getContainerSize(); i++ )
    {
        for ( int j = 0; j < tetr.getContainerSize(); j++ )
        {
            if ( tetr.getCellState( i, j ) != 0 )
            {
                pb.modifyCell( i + tetr.getRow(), j + tetr.getCol(), tetr.getCellState( i, j ) );
            }
        }
    }

    pb.clearCompletedRow( tetr.getRow() + tetr.getContainerSize() - 1, tetr.getRow() );
}

Tetrimino generateTetrimino()
{
    int selection = rand() % 7 + 1;
    Tetrimino newTetr(selection);
    return newTetr;
}

void handlingKeyPress( SDL_Event &e, const PlayBoard &pb, Tetrimino &tetr )
{
    if ( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        // cout << SDL_GetKeyName(e.key.keysym.sym) << endl;
        switch( e.key.keysym.sym )
        {
            //Drops
            case SDLK_DOWN:
            case SDLK_SPACE:
                tetr.dropPiece( pb, e.key.keysym.sym == SDLK_SPACE );
                break;
            
            //Moves left or right
            case SDLK_LEFT:
            case SDLK_RIGHT:
                tetr.movePieceHorizontally( pb, e.key.keysym.sym == SDLK_RIGHT );
                break;
            
            //Rotates clockwise
            case SDLK_UP:
            case SDLK_x:
            //Rotates counterclockwise
            case SDLK_z:
                tetr.rotatePiece( pb, e.key.keysym.sym != SDLK_z );
                break;
        }
    }
}