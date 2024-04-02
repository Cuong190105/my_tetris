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

int handlePauseMenu()
{
    SDL_Rect overlay { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
    SDL_RenderFillRect( renderer, &overlay );
    renderText( "GAME PAUSED", WINDOW_WIDTH / 2, TILE_WIDTH * 8, false, CENTER, MIDDLE, 2 );
    enum PauseMenuButton { CONTINUE, SETTINGS, QUIT };
    SDL_Rect buttons[3];
    string content[] = { "CONTINUE", "SETTINGS", "BACK TO MENU" };
    buttons[CONTINUE] = SDL_Rect { TILE_WIDTH * 28, TILE_WIDTH * 16, TILE_WIDTH * 8, TILE_WIDTH * 2};
    buttons[SETTINGS] = SDL_Rect { TILE_WIDTH * 28, TILE_WIDTH * 19, TILE_WIDTH * 8, TILE_WIDTH * 2 };
    buttons[QUIT] = SDL_Rect { TILE_WIDTH * 28, TILE_WIDTH * 22, TILE_WIDTH * 8, TILE_WIDTH * 2 };
    int mouse_x, mouse_y;
    int activeButton = -1;
    SDL_GetMouseState( &mouse_x, &mouse_y );
    if ( mouse_x >= TILE_WIDTH * 28 && mouse_x <= TILE_WIDTH * 36)
    {
        for ( int i = 0; i < 3; i++ )
        {
            if ( mouse_y >= buttons[i].y && mouse_y <= buttons[i].y + TILE_WIDTH * 2) { activeButton = i; break; }
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if ( activeButton == i)
        {
            SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
            SDL_RenderFillRect( renderer, &buttons[i] );
            renderText( content[i], buttons[i].x + buttons[i].w / 2, buttons[i].y + buttons[i].h / 2, false, CENTER, MIDDLE, 1, {0, 0, 0});
        }
        else
        {
            SDL_SetRenderDrawColor( renderer, 255, 255, 255, 0 );
            SDL_RenderFillRect( renderer, &buttons[i] );
            renderText( content[i], buttons[i].x + buttons[i].w / 2, buttons[i].y + buttons[i].h / 2, false, CENTER, MIDDLE, 1, {255, 255, 255});
        }
    }
    return activeButton;
}

void gameHandler( int players, int gameMode, int mod[4], int &scene, bool &transIn )
{
    bool play = true;
    while ( play )
    {
        vector<Tetromino> Tqueue;
        Uint32 startMark, pauseMark, resumeMark;
        bool start = false, win = false;
        enum pauseMenuOption { CONTINUE, SETTINGS, QUIT_PLAY };
        loadRandomBackground();
        if ( players == 1 )
        {
            Player player ( mod[LEVEL], gameMode, ( WINDOW_WIDTH - BOARD_WIDTH ) / 2, ( WINDOW_HEIGHT - BOARD_HEIGHT ) / 2 ) ;
            int isDrawn = true;
            if ( gameMode == MASTER ) mod[LINECAP] = 300;
            else if ( gameMode == CLASSIC || gameMode == BLITZ ) mod[LINECAP] = -1;
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
                        start = displayCountdown(player.getX(), player.getY(), BOARD_WIDTH, BOARD_HEIGHT, (scene != PAUSE ? startMark : resumeMark));
                        if ( start )
                        {
                            if ( scene == PAUSE ) { startMark += SDL_GetTicks() - pauseMark; player.setTimeMark( pauseMark ); scene = INGAME; }
                            else startMark = SDL_GetTicks();
                            playBackgroundMusic((players > 1 || gameMode == MASTER) ? FAST_THEME : CHILL_THEME);
                        }
                    }
                }
                else if ( scene != PAUSE )
                {
                    generateTetromino( Tqueue );
                    player.ingameProgress( Tqueue, isDrawn, scene );
                    if ( isDrawn ) {
                        Tqueue.erase(Tqueue.begin());
                        isDrawn = false;
                    }
                    renderStatistics( player, startMark, gameMode == TIME ? mod[TIME] : 0, mod[LINECAP] );
                    player.displayCurrentTetromino();
                    player.displayHeldTetromino();
                    player.displayBonus();
                    switch( gameMode )
                    {
                        case SPRINT:
                            if ( player.getLine() >= mod[LINECAP] ) {player.terminateGame(); win = true;}
                            break;
                        case BLITZ:
                            if ( SDL_GetTicks() - startMark >= mod[TIME] * 60000 ) {player.terminateGame();win = true;}
                            break;
                        case MASTER:
                            if ( player.getLine() >= 300 ) {player.terminateGame();win = true;}
                            else if ( player.getLevel() == player.getLine() / 10 ) player.setLevel( player.getLine() / 10 + 1 );
                            break;
                        case CLASSIC:
                        {
                            int a = mod[LEVEL] > 9, b = a * 16 > mod[LEVEL] ? 0 : mod[LEVEL] - a * 16;
                            int tmp = (a*10+b) * 10;
                            if ( player.getLine() > tmp ) player.setLevel( mod[LEVEL] + (player.getLine() - tmp) / 10 );
                            break;
                        }
                        case MYSTERY:
                            enum mysteryEvent {};
                            break;
                    }
                    if ( scene == PAUSE ) { pauseMark = SDL_GetTicks(); stopMusic( false ); }
                }
                else
                {
                    renderStatistics( player, startMark + SDL_GetTicks() - pauseMark, gameMode == TIME ? mod[TIME] : 0, mod[LINECAP] );
                    player.displayCurrentTetromino();
                    player.displayHeldTetromino();
                    player.displayBoard();
                    SDL_Event e;
                    int state = handlePauseMenu();
                    while ( SDL_PollEvent( &e ) )
                    {
                        if ( e.type == SDL_QUIT )
                        {
                            play = false;
                            player.terminateGame();
                            scene = QUIT;
                        }
                        else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE ) start = false;
                        else if ( state != -1 && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT )
                        {
                            switch(state)
                            {
                                case CONTINUE:
                                    start = false;
                                    resumeMark = SDL_GetTicks();
                                    break;
                                case QUIT_PLAY:
                                    play = false;
                                    player.terminateGame();
                                    scene = SOLO_MENU;
                                    break;
                            }
                            playSfx( SELECT );
                        }
                    }
                }
                SDL_RenderPresent( renderer );
            }
            if ( scene != QUIT && scene != SOLO_MENU )
            {
                Uint32 endMark = SDL_GetTicks();
                stopMusic( false );
                if ( win ) playSfx(VICTORY);
                else playSfx(GAMEOVER);

                while ( SDL_GetTicks() - endMark <= 5000)
                {
                    clearScreen();
                    bgImage.render();
                    player.displayBoard();
                    player.displayTetrominoQueue( Tqueue );
                    renderStatistics( player, startMark + SDL_GetTicks() - endMark, gameMode == TIME ? mod[TIME] : 0, mod[LINECAP] );
                    player.displayHeldTetromino();
                    if ( win )
                    {
                        if ( gameMode == BLITZ ) renderText( "TIME'S UP!", player.getX() + BOARD_WIDTH / 2, player.getY() + BOARD_HEIGHT / 2, true, CENTER, MIDDLE, 3);
                        else renderText( "LEVEL COMPLETED!", player.getX() + BOARD_WIDTH / 2, player.getY() + BOARD_HEIGHT / 2, true, CENTER, MIDDLE, 2);
                    }
                    SDL_RenderPresent( renderer );
                }

                string time = "";
                int time_in_seconds = ( endMark - startMark ) / 1000;
                if ( time_in_seconds / 60 < 10) time += '0';
                time += to_string( time_in_seconds / 60 ) + ":";
                if ( time_in_seconds % 60 < 10) time += '0';
                time += to_string( time_in_seconds % 60 );

                endMark = SDL_GetTicks();
                while ( win && SDL_GetTicks() - endMark <= 10000 )
                {
                    clearScreen();
                    bgImage.render();
                    if (SDL_GetTicks() - endMark <= 9250) renderResultScreen( player, endMark, time );
                    else renderResultScreen( player, endMark + 9250, time, true );
                    SDL_RenderPresent( renderer );
                }
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
        if ( scene != QUIT && scene != SOLO_MENU && scene != MULTI_MENU )
        {
            bool retryLoop = true;
            SDL_Event e;
            while(retryLoop)
            {
                clearScreen();
                bgImage.render();
                int activeButton = renderRetryScreen( retryLoop, scene );
                SDL_RenderPresent( renderer );
                while (SDL_PollEvent( &e ))
                {
                    if (e.type == SDL_QUIT) 
                    {
                        scene = QUIT;
                        retryLoop = false;
                        play = false;
                    }
                    else if (activeButton != -1 && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
                    {
                        retryLoop = false;
                        play = activeButton == 0;
                    }
                }
            }
        }
        else play = false;
        while ( scene != QUIT && !transIn )
        {
            renderTransition( transIn );
            SDL_RenderPresent( renderer );
        }
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
    loadMenuElements();
    Texture title;
    title.loadFromFile("src/media/img/game_title.png");
    int activeButton = -1;
    int mouse_x, mouse_y;
    SDL_Event event;
    int foregroundAlphaMod = 10;
    vector<Tetromino> floating;
    bool adjusted = false;
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
                        if (activeButton != -1)
                        {
                            changeMenu = activeButton + 1;
                            playSfx( SELECT );
                        }
                        break;
                    case SOLO_MENU:
                        if ( backActive ) 
                        {
                            changeMenu = MAIN_MENU;
                            playSfx( SELECT );
                        }
                        else if ( activeButton != -1 )
                        {
                            players = 1;
                            gameMode = activeButton;
                            changeMenu = SET_RULES;
                            playSfx( SELECT );
                        }
                        break;
                    case SET_RULES:
                        if ( backActive ) { changeMenu = (players == 1) ? SOLO_MENU : MULTI_MENU; backActive = false; playSfx( SELECT ); }
                        else if ( activeButton != 0 ) { adjusted = true; playSfx( SELECT ); }
                        else if ( startActive ) { changeMenu = INGAME; scene = changeMenu; playSfx( SELECT ); }
                    case MULTI_MENU:
                        // gameHandler( false, activeButton );
                        // break;
                    case SETTINGS:
                        if ( backActive ) { changeMenu = MAIN_MENU; playSfx( SELECT ); }
                        break;
                    case QUIT:
                        break;
                }
            }
        }
        
    }
    stopMusic( true );
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
