#include <algorithm>
#include <iostream>
#include <SDL.h>
#include <cstdlib>
#include <random>
#include "logic.hpp"
using namespace std;

void generateTetrimino( vector<Tetrimino> &Tqueue )
{
    //7-bag randomization (Takes all 7 types of tetrimino, shuffles them, then pushes them into the queue).
    //This method guarantees every unique pieces will be spawn within 7 turns, minimizes the missing/repetition
    //of a piece in some cases.
    while ( Tqueue.size() <= 7 ) {
        vector<int> v { I_PIECE, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE };
        random_device rd;
        mt19937 g(rd());
        shuffle(v.begin(), v.end(), g);
        for ( int i = 0; i < 7; i++ )
        {
            Tqueue.push_back( Tetrimino( v[i] ) );
        }
    }
}

void pullNewTetrimino( vector<Tetrimino> &Tqueue, Tetrimino &tetr )
{
    generateTetrimino( Tqueue );
    tetr = Tqueue[0];
    Tqueue.erase(Tqueue.begin());
}

void holdPiece( Tetrimino &tetr, Tetrimino &hold, bool &holdable )
{
    if ( holdable )
    {
        int tmp = hold.getType();
        hold = Tetrimino(tetr.getType());
        tetr = Tetrimino(tmp);
        holdable = false;
    }
}

void handlingKeyPress( SDL_Event &e, PlayBoard &pb, Tetrimino &tetr, Tetrimino &hold, bool &holdable )
{
    if ( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        // cout << SDL_GetKeyName(e.key.keysym.sym) << endl;
        switch( e.key.keysym.sym )
        {
            //Drops
            case SDLK_DOWN:
            case SDLK_SPACE:
                tetr.dropPiece( pb, e.key.keysym.sym == SDLK_SPACE, holdable );
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
            case SDLK_c:
                holdPiece( tetr, hold, holdable );
                break;
        }
    }
}