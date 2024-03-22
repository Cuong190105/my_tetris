#include <algorithm>
#include <iostream>
#include <random>
#include "logic.hpp"

using namespace std;

SDL_Event event;

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

void mainMenu( int &scene, int &activeButton )
{
    renderMenuBackground();
    // renderFloatingTetromino();
    // renderGameTitle();

    int mouse_x, mouse_y;
    SDL_GetMouseState( &mouse_x, &mouse_y );
    renderMainMenuButton( mouse_x, mouse_y, activeButton );
}

void gameManager( int &scene )
{
    //Current game scene
    int activeButton = -1;
    switch(scene)
    {
        case MAIN_MENU:
            mainMenu( scene, activeButton );
            break;
        case SOLO_MENU:
            // soloMenu( scene );
            break;
        case MULTI_MENU:
            // multiMenu( scene );
            break;
        case SETTINGS:
            // settingMenu( scene );
            break;
        case QUIT:
            break;
    }

    //Handling input events
    while( SDL_PollEvent( &event ) > 0 && scene != QUIT )
    {
        switch( event.type )
        {
            case SDL_QUIT:
                scene = QUIT;
                break;
            case SDL_MOUSEBUTTONUP:
                handlingMouseClick( scene, activeButton );
                break;
            case SDL_KEYDOWN:
                // handlingKeyPress
                break;
        }
    }
}

void handlingMouseClick( int &scene, int activeButton )
{
    if ( activeButton != -1 ) scene = activeButton + 1;
}