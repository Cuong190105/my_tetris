#include <algorithm>
#include <iostream>
#include <SDL.h>
#include <random>
#include "logic.hpp"
#include "rendering.hpp"
using namespace std;

void generateTetromino( vector<Tetromino> &Tqueue )
{
    //7-bag randomization (Takes all 7 types of tetromino, shuffles them, then pushes them into the queue).
    //This method guarantees every unique pieces will be spawn within 7 turns, minimizes the missing/repetition
    //of a piece in some cases.
    while ( Tqueue.size() <= 7 ) {
        vector<int> v { I_PIECE, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE };
        random_device rd;
        mt19937 g(rd());
        shuffle(v.begin(), v.end(), g);
        for ( int i = 0; i < 7; i++ )
        {
            Tqueue.push_back( Tetromino( v[i] ) );
        }
    }
}

void handlingKeyPress( SDL_Event &e, Player& player )
{
    if ( e.type == SDL_KEYDOWN )
    {
        // cout << SDL_GetKeyName(e.key.keysym.sym) << endl;
        switch( e.key.keysym.sym )
        {
            //Drops
            case SDLK_DOWN:
            case SDLK_SPACE:
                player.dropPiece( e.key.keysym.sym == SDLK_SPACE );
                break;
            
            //Moves left or right
            case SDLK_LEFT:
            case SDLK_RIGHT:
                player.movePieceHorizontally( e.key.keysym.sym == SDLK_RIGHT );
                break;
            
            //Rotates clockwise
            case SDLK_UP:
            case SDLK_x:
            //Rotates counterclockwise
            case SDLK_z:
                player.rotatePiece( e.key.keysym.sym != SDLK_z );
                break;
            case SDLK_c:
                player.swapHoldPiece();
                break;
        }
    }
}

void ingameProgress()
{
    //Initialize game & piece bag;
    Player player;
    bool gameOver = false;
    SDL_Event ingameEvent;
    vector<Tetromino> Tqueue;
    while ( !gameOver )
    {
        clearScreen();
        if ( !gameOver && player.tetr.getType() == 0 )
        {
            generateTetromino( Tqueue );

            player.pullNewTetromino( Tqueue );
            Tqueue.erase( Tqueue.begin() );
            if ( player.checkCollision( player.tetr ) ) {
                gameOver = true;
            }
        }
        
        player.gravityPull();

        //Handle events
        while ( SDL_PollEvent(&ingameEvent) != 0 ) 
        {
            //Quitting game event
            if (  ingameEvent.type == SDL_QUIT )
            {
                gameOver = true;
                break;
            }

            if ( !gameOver ) handlingKeyPress( ingameEvent, player );
        }
        renderFrame( player, Tqueue );
    }
}

void gameManager()
{
    enum gameState {MAIN_MENU, SOLO_MENU, MULTI_MENU, SETTINGS, QUIT};

}