#include <algorithm>
#include <iostream>
#include <random>
#include "logic.hpp"

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

void gameHandler( int players, int gameMode, int mod[4], int &scene, bool &transIn )
{
    // renderTransition( true );
    vector<Tetromino> Tqueue;
    Uint32 startMark;
    bool start = false;
    loadRandomBackground();
    if ( players == 1 )
    {
        Player player ( mod[LEVEL], gameMode, ( WINDOW_WIDTH - BOARD_WIDTH ) / 2, ( WINDOW_HEIGHT - BOARD_HEIGHT ) / 2 ) ;
        int isDrawn = true;
        while ( !player.isGameOver() )
        {
            clearScreen();
            bgImage.render();
            player.displayBoard();
            player.displayTetrominoQueue( Tqueue );
            if ( !start ) 
            {
                if ( transIn )
                {
                    renderTransition( transIn );
                    if ( !transIn ) startMark = SDL_GetTicks();
                }
                else
                {
                    start = displayCountdown(player.getX(), player.getY(), BOARD_WIDTH, BOARD_HEIGHT, startMark);
                    if ( start ) startMark = SDL_GetTicks();
                }
            }
            else 
            {
                renderStatistics( player, startMark, gameMode == TIME ? mod[TIME] : 0 );
                generateTetromino( Tqueue );
                player.ingameProgress( Tqueue, isDrawn, scene );
                if ( isDrawn ) {
                    Tqueue.erase(Tqueue.begin());
                    isDrawn = false;
                }
                player.displayCurrentTetromino();
                player.displayHeldTetromino();
                switch( gameMode )
                {
                    case SPRINT:
                        if ( player.getLine() >= mod[LINECAP] ) player.terminateGame();
                        break;
                    case BLITZ:
                        if ( SDL_GetTicks() - startMark >= mod[TIME] * 60000 ) player.terminateGame();
                        break;
                    case MASTER:
                        if ( player.getLine() >= 300 ) player.terminateGame();
                        else if ( player.getLevel() == player.getLine() / 10 ) player.setLevel( player.getLine() / 10 + 1 );
                        break;
                    case CLASSIC:
                    {
                        int tmp = player.getLine() / 10;
                        tmp = tmp % 10 + tmp / 10 * 16;
                        if ( tmp > mod[LEVEL]) player.setLevel( tmp + 1 );
                        break;
                    }
                    case MYSTERY:
                        enum mysteryEvent {};
                        break;
                }
            }
            SDL_RenderPresent( renderer );
        }
    }
    else
    {
        vector<Player> player;
        vector<int> queuePosition( players, 0 );
        for (int i = 0; i < players; i++)
        {
            int x = ( (2 * i + 1 ) * WINDOW_WIDTH / players - BOARD_WIDTH) / 2, y = (WINDOW_HEIGHT - BOARD_HEIGHT) / 2;
            player.push_back( Player( mod[LEVEL], gameMode, x, y ) );
        }
    }
    while ( scene != QUIT && !transIn )
    {
        renderTransition( transIn );
        SDL_RenderPresent( renderer );
    }
}

int adjustmentButton( int x, int y, bool disableLeft, bool disableRight )
{
    int mouse_x, mouse_y;
    renderAdjustmentButton( x, y, disableLeft, disableRight );
    SDL_GetMouseState( &mouse_x, &mouse_y );
    if ( mouse_y >= y && mouse_y <= y + TILE_WIDTH )
    {
        if ( (!disableLeft) && (mouse_x >= x + LEFT_ADJUSTMENTBUTTON_X) && (mouse_x <= x + LEFT_ADJUSTMENTBUTTON_X + TILE_WIDTH) )
        {
            return -1;
        }
        if ( (!disableRight) && (mouse_x >= x + RIGHT_ADJUSTMENTBUTTON_X) && (mouse_x <= x + RIGHT_ADJUSTMENTBUTTON_X + TILE_WIDTH) )
        {
            return 1;
        }
    }
    return 0;
}

void settingRules( bool isSolo, int gameMode, int &activeButton, bool &adjusted, int mod[4] )
{
    SDL_Rect rect { TILE_WIDTH * 12, TILE_WIDTH * 6, TILE_WIDTH * 40, TILE_WIDTH * 24 };
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
    SDL_RenderFillRect( renderer, &rect);
    int mouse_x, mouse_y;
    if ( isSolo )
    {
        renderText( soloGameModeName[gameMode], WINDOW_WIDTH / 2, TILE_WIDTH * 12, true, CENTER, BOTTOM, 4, SDL_Color {255, 255, 255} );
        mod[ACTIVATE_MYSTERY] = 0;
        if ( adjusted )
        {
            if ( abs(activeButton) == LINECAP + 1 ) mod[LINECAP] +=  10 * (abs(activeButton) / activeButton);
            else mod[abs(activeButton) - 1] +=  activeButton != 0 ? abs(activeButton) / activeButton : 0;
            adjusted = false;
        }
        switch( gameMode )
        {
            case CLASSIC:
                renderText( "INITIAL SPEED LEVEL" , WINDOW_WIDTH / 2, TILE_WIDTH * 14, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[LEVEL] ), WINDOW_WIDTH / 2, TILE_WIDTH * 16, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, TILE_WIDTH * 16, mod[LEVEL] == 1, mod[LEVEL] == 19) * (LEVEL + 1);
                break;
            case SPRINT:
                mod[LEVEL] = 1;
                if (mod[LINECAP] < 40 || mod[LINECAP] > 100) {mod[LINECAP] = 40;}
                renderText( "SET LINE TARGET" , WINDOW_WIDTH / 2, TILE_WIDTH * 14, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[LINECAP] ), WINDOW_WIDTH / 2, TILE_WIDTH * 16, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, TILE_WIDTH * 16, mod[LINECAP] == 40, mod[LINECAP] == 100 ) * (LINECAP + 1);
                break;
            case BLITZ:
                mod[LEVEL] = 1;
                renderText( "SET TIME LIMIT" , WINDOW_WIDTH / 2, TILE_WIDTH * 14, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[TIME] ), WINDOW_WIDTH / 2, TILE_WIDTH * 16, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, TILE_WIDTH * 16, mod[TIME] == 2, mod[TIME] == 10 ) * (TIME + 1);
                break;
            case MASTER:
                renderText( "INITIAL SPEED LEVEL" , WINDOW_WIDTH / 2, TILE_WIDTH * 14, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                renderText( "M" + to_string( mod[LEVEL] ), WINDOW_WIDTH / 2, TILE_WIDTH * 16, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, TILE_WIDTH * 16, mod[LEVEL] == 1, mod[LEVEL] == 30 ) * (LEVEL + 1);
                break;
            case MYSTERY:
                mod[ACTIVATE_MYSTERY] = 1;
                if (mod[LINECAP] != -1 && mod[LINECAP] != 150) mod[LINECAP] = 150;
                renderText( "GO ENDLESS?" , WINDOW_WIDTH / 2, TILE_WIDTH * 14, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                renderText( mod[LINECAP] == -1 ? "Endless" : to_string( mod[LINECAP] ), WINDOW_WIDTH / 2, TILE_WIDTH * 16, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, TILE_WIDTH * 16, mod[LINECAP] == 150, mod[LINECAP] == -1 ) * (LINECAP + 1);

                renderText( "INITIAL SPEED LEVEL" , WINDOW_WIDTH / 2, TILE_WIDTH * 20, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[LEVEL] ), WINDOW_WIDTH / 2, TILE_WIDTH * 22, false, CENTER, TOP, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, TILE_WIDTH * 22, mod[LEVEL] == 1, mod[LEVEL] == 15 ) * (LEVEL + 1);
                break;
        }
    }
    else
    {
        renderText( soloGameModeName[gameMode], 0, TILE_WIDTH * 10, true, CENTER, TOP, 4, SDL_Color {255, 255, 255} );
    }
}

void menuManager( int &scene, bool &transIn, int &players,  int &gameMode, int mod[4]  )
{
    Texture title;
    title.loadFromFile("src/media/img/game_title.png");
    int activeButton = -1;
    int mouse_x, mouse_y;
    SDL_Event event;
    int foregroundAlphaMod = 10;
    vector<Tetromino> floating;
    bool goSettingRules = false, adjusted = false;
    int changeMenu = scene;
    Uint32 animationMark = SDL_GetTicks();
    const int ANIMATION_DURATION = 500;
    while ( scene != INGAME && scene != QUIT )
    {
        clearScreen();
        renderMenuBackground();
        generateTetromino( floating );
        renderFloatingTetromino( floating );

        SDL_Texture *foreground = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT );
        SDL_SetRenderTarget( renderer, foreground );
        SDL_SetTextureBlendMode( foreground, SDL_BLENDMODE_BLEND );
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
        SDL_RenderClear( renderer );
        SDL_GetMouseState( &mouse_x, &mouse_y );
        bool backActive = false, startActive = false;
        switch( scene )
        {
            case MAIN_MENU:
                renderGameTitle( title );
                renderMainMenuButton( mouse_x, mouse_y, activeButton );
                break;
            case SOLO_MENU:
                renderSoloMenu( mouse_x, mouse_y, activeButton );
                // else 
                // {
                //     players = 1;
                //     settingRules( true, gameMode, activeButton, adjusted, mod );
                //     startActive = handleStartButton( mouse_x, mouse_y, WINDOW_WIDTH / 2, TILE_WIDTH * 26 );
                // }
                backActive = handleBackButton( mouse_x, mouse_y );
                break;
            case SET_RULES:
                settingRules( true, gameMode, activeButton, adjusted, mod );
                startActive = handleStartButton( mouse_x, mouse_y, WINDOW_WIDTH / 2, TILE_WIDTH * 26 );
                backActive = handleBackButton( mouse_x, mouse_y );
                break;

            case MULTI_MENU:
            case SETTINGS:
                renderText( "Under Construction", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, true, CENTER, MIDDLE, 3, SDL_Color {255, 255, 255} );
                backActive = handleBackButton( mouse_x, mouse_y );
                break;
            default:
                scene = QUIT;
        }
        SDL_SetRenderTarget( renderer, NULL );

        if ( changeMenu != QUIT )
        {
            if ( scene != changeMenu )
            {
                if ( SDL_GetTicks() - animationMark > ANIMATION_DURATION / 60 )
                {
                    foregroundAlphaMod --;
                    if ( foregroundAlphaMod == 0 )
                    {
                        scene = changeMenu;
                    }
                    animationMark = SDL_GetTicks();
                }
            }
            else if ( foregroundAlphaMod < 10 && SDL_GetTicks() - animationMark > ANIMATION_DURATION / 60 )
            {
                animationMark = SDL_GetTicks();
                foregroundAlphaMod ++;
            }
            if (foregroundAlphaMod < 10) SDL_SetTextureAlphaMod( foreground, 255 * foregroundAlphaMod / 10 );
        } else scene = QUIT;

        SDL_RenderCopy( renderer, foreground, NULL, NULL );
        SDL_DestroyTexture( foreground );
        foreground = NULL;
        if ( transIn ) renderTransition( transIn );
        SDL_RenderPresent( renderer );
        while ( SDL_PollEvent( &event ) > 0 )
        {
            if ( event.type == SDL_QUIT ) {scene = QUIT; break;}
            else if ( event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT )
            {
                switch( scene )
                {
                    case MAIN_MENU:
                        if (activeButton != -1) changeMenu = activeButton + 1;
                        break;
                    case SOLO_MENU:
                        if ( backActive ) 
                        {
                            changeMenu = MAIN_MENU;
                        }
                        else if ( activeButton != -1 )
                        {
                            players = 1;
                            gameMode = activeButton;
                            changeMenu = SET_RULES;
                        }
                        break;
                    case SET_RULES:
                        if ( backActive ) { changeMenu = (players == 1) ? SOLO_MENU : MULTI_MENU; backActive = false; }
                        else if ( activeButton != 0 ) adjusted = true;
                        else if ( startActive ) { changeMenu = INGAME; scene = changeMenu; }
                    case MULTI_MENU:
                        // gameHandler( false, activeButton );
                        // break;
                    case SETTINGS:
                        if ( backActive ) changeMenu = MAIN_MENU;
                        break;
                    case QUIT:
                        break;
                }
            }
        }
        
    }
    while ( scene != QUIT && !transIn ) {
        renderTransition( transIn );
        SDL_RenderPresent ( renderer );
    }
}

void taskManager()
{
    //Current game scene
    int mod[] = { 1, 40, 2, false };
    int players, gameMode;
    bool transIn = true;
    int scene = MAIN_MENU;
    //Game loop
    while ( scene != QUIT )
    {
        switch(scene)
        {
            case MAIN_MENU:
            case SOLO_MENU:
            case MULTI_MENU:
            case SETTINGS:
                menuManager( scene, transIn, players, gameMode, mod );
                break;
            case INGAME:
                gameHandler( players, gameMode, mod, scene, transIn );
                if ( scene != QUIT )
                {
                    if (players == 1) scene = SOLO_MENU;
                    else if (players > 1) scene = MULTI_MENU;
                }
                break;
            case QUIT:
                scene = QUIT;
                break;
        }
    }
}
