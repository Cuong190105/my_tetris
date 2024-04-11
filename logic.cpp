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
    renderText( "GAME PAUSED", WINDOW_WIDTH / 2, LENGTH_UNIT * 8, false, CENTER, MIDDLE, 2 );
    enum PauseMenuButton { CONTINUE, RETRY, SETTINGS, QUIT };
    SDL_Rect buttons[4];
    string content[] = { "CONTINUE", "RETRY", "SETTINGS", "BACK TO MENU" };
    buttons[CONTINUE] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 16, LENGTH_UNIT * 8, LENGTH_UNIT * 2};
    buttons[RETRY] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 19, LENGTH_UNIT * 8, LENGTH_UNIT * 2 };
    buttons[SETTINGS] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 22, LENGTH_UNIT * 8, LENGTH_UNIT * 2 };
    buttons[QUIT] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 25, LENGTH_UNIT * 8, LENGTH_UNIT * 2};
    int mouse_x, mouse_y;
    int activeButton = -1;
    SDL_GetMouseState( &mouse_x, &mouse_y );
    if ( mouse_x >= LENGTH_UNIT * 28 && mouse_x <= LENGTH_UNIT * 36)
    {
        for ( int i = 0; i < 4; i++ )
        {
            if ( mouse_y >= buttons[i].y && mouse_y <= buttons[i].y + LENGTH_UNIT * 2) { activeButton = i; break; }
        }
    }
    for (int i = 0; i < 4; i++)
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
    SDL_Texture *foreground = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT );
    SDL_SetTextureBlendMode( foreground, SDL_BLENDMODE_BLEND );
    while ( play )
    {
        vector<Tetromino> Tqueue;
        Uint32 startMark, pauseMark, resumeMark;
        bool start = false, win = false;
        enum pauseMenuOption { CONTINUE, RETRY, SETTINGS, QUIT_PLAY };
        loadRandomBackground();
        if ( players == 1 )
        {
            Player player ( mod[LEVEL], gameMode, ( WINDOW_WIDTH - BOARD_WIDTH ) / 2, ( WINDOW_HEIGHT - BOARD_HEIGHT ) / 2 ) ;
            int isDrawn = true;
            if ( gameMode == MASTER ) mod[LINECAP] = 300;
            else if ( gameMode == CLASSIC ) mod[LINECAP] = -1;
            else if ( gameMode == BLITZ ) mod[LINECAP] = 3;
            while ( !player.isGameOver() )
            {
                clearScreen();
                bgImage.render();
                if ( start )
                {
                    SDL_SetRenderTarget( renderer, foreground );
                    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
                    SDL_RenderClear( renderer );
                }
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
                        if ( start )
                        {
                            startMark = SDL_GetTicks();
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
                            // if ( player.getLine() >= mod[LINECAP] ) {player.terminateGame(); win = true;}
                            if ( player.getLine() >= 1 ) {player.terminateGame(); win = true;}
                            break;
                        case BLITZ:
                            if ( SDL_GetTicks() - startMark >= mod[TIME] * 60000 ) {player.terminateGame();win = true;}
                            if ( player.getLine() >= mod[LINECAP] )
                            {
                                player.setLevel(player.getLevel() + 1);
                                mod[LINECAP] += player.getLevel() * 2 + 1;
                            }
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
                            if ( player.getLine() / 10 + 1 > player.getLevel() ) player.setLevel( player.getLevel() + 1 );
                            const string eventName[] = {"UPSIDE DOWN", "UNSTABLE", "BOMB", "GIANT", "GARBAGE", "CORRUPTED", "HORIZONTAL SHIFT"};
                            if (player.getMysteryEvent() > -1 && SDL_GetTicks() - player.getMysteryMark() < 3000)
                            {
                                renderText( eventName[player.getMysteryEvent()],
                                            player.getX() - LENGTH_UNIT,
                                            player.getY() + BOARD_HEIGHT - LENGTH_UNIT * 8,
                                            false, RIGHT, BOTTOM, 1.25 );
                            }
                            player.handleMysteryEvents( Tqueue );
                            if (mod[LINECAP] == 150 && player.getLine() >= mod[LINECAP] ) {player.terminateGame(); win = true;}
                            break;
                    }
                    SDL_SetRenderTarget( renderer, NULL );
                    if ( player.getMysteryEvent() == UPSIDE_DOWN ) SDL_RenderCopyEx( renderer, foreground, NULL, NULL, 180, NULL, SDL_FLIP_NONE );
                    else SDL_RenderCopy( renderer, foreground, NULL, NULL );
                    if ( scene == PAUSE ) { pauseMark = SDL_GetTicks(); pauseMusic(); }
                }
                else
                {
                    renderStatistics( player, startMark + SDL_GetTicks() - pauseMark, gameMode == TIME ? mod[TIME] : 0, mod[LINECAP] );
                    player.displayHeldTetromino();
                    player.displayBoard();
                    player.displayCurrentTetromino();
                    SDL_SetRenderTarget( renderer, NULL );
                    if ( gameMode == MYSTERY && player.getMysteryEvent() == UPSIDE_DOWN ) SDL_RenderCopyEx( renderer, foreground, NULL, NULL, 180, NULL, SDL_FLIP_NONE );
                    else SDL_RenderCopy( renderer, foreground, NULL, NULL );
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
                        else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE ) scene = INGAME;
                        else if ( state != -1 && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT )
                        {
                            switch(state)
                            {
                                case CONTINUE:
                                    scene = INGAME;
                                    break;
                                case RETRY:
                                    player.terminateGame();
                                    start = false;
                                    stopMusic( false );
                                    scene = INGAME;
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
                    if ( start && scene == INGAME )
                    {
                        resumeMusic();
                        player.setTimeMark( pauseMark );
                        startMark += SDL_GetTicks() - pauseMark;
                    }
                }
                SDL_RenderPresent( renderer );
            }
            if ( start && scene != QUIT && scene != SOLO_MENU )
            {
                Uint32 endMark = SDL_GetTicks();
                stopMusic( false );
                if ( win ) playSfx(VICTORY);
                else playSfx(GAMEOVER);

                while ( SDL_GetTicks() - endMark <= 5000)
                {
                    clearScreen();
                    bgImage.render();
                    SDL_SetRenderTarget( renderer, foreground );
                    clearScreen();
                    player.displayBoard();
                    player.displayTetrominoQueue( Tqueue );
                    renderStatistics( player, startMark + SDL_GetTicks() - endMark, gameMode == TIME ? mod[TIME] : 0, mod[LINECAP] );
                    player.displayHeldTetromino();
                    if ( win )
                    {
                        if ( gameMode == BLITZ ) renderText( "TIME'S UP!", player.getX() + BOARD_WIDTH / 2, player.getY() + BOARD_HEIGHT / 2 - LENGTH_UNIT * 3, true, CENTER, MIDDLE, 3);
                        else renderText( "LEVEL COMPLETED!", player.getX() + BOARD_WIDTH / 2, player.getY() + BOARD_HEIGHT / 2 - LENGTH_UNIT * 3, true, CENTER, MIDDLE, 2);
                    }
                    SDL_SetRenderTarget( renderer, NULL );
                    SDL_RenderCopy( renderer, foreground, NULL, NULL );
                    SDL_RenderPresent( renderer );
                }

                string time = "";
                int time_in_seconds = ( endMark - startMark ) / 1000;
                if ( time_in_seconds / 60 < 10) time += '0';
                time += to_string( time_in_seconds / 60 ) + ":";
                if ( time_in_seconds % 60 < 10) time += '0';
                time += to_string( time_in_seconds % 60 );


                endMark = SDL_GetTicks();
                while ( SDL_GetTicks() - endMark < 500 )
                {
                    clearScreen();
                    bgImage.render();
                    SDL_SetRenderTarget( renderer, NULL );
                    SDL_SetTextureAlphaMod( foreground, 255 * max((int)( 500 - SDL_GetTicks() + endMark ), 0) / 500 );
                    SDL_RenderCopy( renderer, foreground, NULL, NULL );
                    SDL_RenderPresent( renderer );
                }

                endMark = SDL_GetTicks();
                while ( win && SDL_GetTicks() - endMark <= 6500 )
                {
                    clearScreen();
                    bgImage.render();
                    SDL_SetRenderTarget( renderer, foreground );
                    clearScreen();
                    renderResultScreen(player, endMark, time );
                    SDL_SetRenderTarget( renderer, NULL );
                    if (SDL_GetTicks() - endMark <= 500)
                    {
                        SDL_SetTextureAlphaMod( foreground, min(255 * (int)(SDL_GetTicks() - endMark) / 500, 255) );
                    }
                    else if ( SDL_GetTicks() - endMark > 6000 )
                    {
                        SDL_SetTextureAlphaMod( foreground, max(255 * (int)(6500 - SDL_GetTicks() + endMark) / 500, 0) );
                    }
                    SDL_RenderCopy( renderer, foreground, NULL, NULL );
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
        if ( start && scene != QUIT && scene != SOLO_MENU && scene != MULTI_MENU )
        {
            bool retryLoop = true;
            SDL_Event e;
            Uint32 mark = SDL_GetTicks();
            while(retryLoop)
            {
                clearScreen();
                bgImage.render();
                SDL_SetRenderTarget( renderer, foreground );
                clearScreen();
                int activeButton = renderRetryScreen( retryLoop, scene );
                SDL_SetRenderTarget( renderer, NULL );
                if (SDL_GetTicks() - mark < 500)
                {
                    SDL_SetTextureAlphaMod( foreground, 255 * (SDL_GetTicks() - mark) / 500 );
                }
                SDL_RenderCopy( renderer, foreground, NULL, NULL );
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
        else if (start) play = false;
        SDL_SetRenderTarget( renderer, NULL );
        while ( scene != QUIT && !transIn )
        {
            bgImage.render();
            SDL_RenderCopy( renderer, foreground, NULL, NULL );
            renderTransition( transIn );
            SDL_RenderPresent( renderer );
        }
    }
    SDL_DestroyTexture( foreground );
    foreground = NULL;
}

int adjustmentButton( int x, int y, bool disableLeft, bool disableRight )
{
    int mouse_x, mouse_y;
    renderAdjustmentButton( x, y, disableLeft, disableRight );
    SDL_GetMouseState( &mouse_x, &mouse_y );
    if ( mouse_y >= y - LENGTH_UNIT / 2 && mouse_y <= y + LENGTH_UNIT / 2 )
    {
        if ( (!disableLeft) && (mouse_x >= x + LEFT_ADJUSTMENTBUTTON_X) && (mouse_x <= x + LEFT_ADJUSTMENTBUTTON_X + LENGTH_UNIT) )
        {
            return -1;
        }
        if ( (!disableRight) && (mouse_x >= x + RIGHT_ADJUSTMENTBUTTON_X) && (mouse_x <= x + RIGHT_ADJUSTMENTBUTTON_X + LENGTH_UNIT) )
        {
            return 1;
        }
    }
    return 0;
}

void settingRules( bool isSolo, int gameMode, int &activeButton, int &adjusted, int mod[4] )
{
    SDL_Rect rect { LENGTH_UNIT * 12, LENGTH_UNIT * 6, LENGTH_UNIT * 40, LENGTH_UNIT * 24 };
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
    SDL_RenderFillRect( renderer, &rect);
    int mouse_x, mouse_y;
    if ( isSolo )
    {
        renderText( soloGameModeName[gameMode], WINDOW_WIDTH / 2, LENGTH_UNIT * 12, true, CENTER, BOTTOM, 4, SDL_Color {255, 255, 255} );
        mod[ACTIVATE_MYSTERY] = 0;
        if ( adjusted > 0 )
        {
            if ( abs(activeButton) == LINECAP + 1 )
            {
                if ( gameMode == MYSTERY ) mod[LINECAP] = activeButton > 0 ? -1 : 150;
                else mod[LINECAP] +=  10 * (abs(activeButton) / activeButton);
            }
            else mod[abs(activeButton) - 1] +=  activeButton != 0 ? abs(activeButton) / activeButton : 0;
            adjusted = false;
        }
        switch( gameMode )
        {
            case CLASSIC:
                renderText( "INITIAL SPEED LEVEL" , WINDOW_WIDTH / 2, LENGTH_UNIT * 14, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[LEVEL] ), WINDOW_WIDTH / 2, LENGTH_UNIT * 17, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 17, mod[LEVEL] == 1, mod[LEVEL] == 19) * (LEVEL + 1);
                break;
            case SPRINT:
                mod[LEVEL] = 1;
                if (mod[LINECAP] < 40 || mod[LINECAP] > 100) {mod[LINECAP] = 40;}
                renderText( "SET LINE TARGET" , WINDOW_WIDTH / 2, LENGTH_UNIT * 14, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[LINECAP] ), WINDOW_WIDTH / 2, LENGTH_UNIT * 17, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 17, mod[LINECAP] == 40, mod[LINECAP] == 100 ) * (LINECAP + 1);
                break;
            case BLITZ:
                mod[LEVEL] = 1;
                renderText( "SET TIME LIMIT" , WINDOW_WIDTH / 2, LENGTH_UNIT * 14, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[TIME] ), WINDOW_WIDTH / 2, LENGTH_UNIT * 17, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 17, mod[TIME] == 2, mod[TIME] == 10 ) * (TIME + 1);
                break;
            case MASTER:
                renderText( "INITIAL SPEED LEVEL" , WINDOW_WIDTH / 2, LENGTH_UNIT * 14, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( "M" + to_string( mod[LEVEL] ), WINDOW_WIDTH / 2, LENGTH_UNIT * 17, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 17, mod[LEVEL] == 1, mod[LEVEL] == 30 ) * (LEVEL + 1);
                break;
            case MYSTERY:
                mod[ACTIVATE_MYSTERY] = 1;
                if (mod[LINECAP] != -1 && mod[LINECAP] != 150) mod[LINECAP] = 150;
                renderText( "GO ENDLESS?" , WINDOW_WIDTH / 2, LENGTH_UNIT * 14, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( mod[LINECAP] == -1 ? "Endless" : to_string( mod[LINECAP] ) + " Lines", WINDOW_WIDTH / 2, LENGTH_UNIT * 17, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                activeButton = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 17, mod[LINECAP] == 150, mod[LINECAP] == -1 ) * (LINECAP + 1);

                renderText( "INITIAL SPEED LEVEL" , WINDOW_WIDTH / 2, LENGTH_UNIT * 20, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( mod[LEVEL] ), WINDOW_WIDTH / 2, LENGTH_UNIT * 23, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                int temp = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 23, mod[LEVEL] == 1, mod[LEVEL] == 15 ) * (LEVEL + 1);
                if ( temp != 0 ) activeButton = temp;
                break;
        }
    }
    else
    {
        renderText( soloGameModeName[gameMode], 0, LENGTH_UNIT * 10, true, CENTER, TOP, 4, SDL_Color {255, 255, 255} );
    }
}

enum MBState { PRESSED = -1, INITIAL, RELEASED };

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
    int adjusted = 0;
    int changeMenu = scene;
    Uint32 animationMark = SDL_GetTicks();
    const int ANIMATION_DURATION = 500;
    while ( scene != INGAME && scene != QUIT )
    {
        clearScreen();;
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
                startActive = handleStartButton( mouse_x, mouse_y, WINDOW_WIDTH / 2, LENGTH_UNIT * 26 );
                backActive = handleBackButton( mouse_x, mouse_y );
                break;

            case SETTINGS:
                gameSettings( scene, activeButton, adjusted );
                backActive = handleBackButton( mouse_x, mouse_y );
                break;
            case MULTI_MENU:
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
            else if ( event.button.button == SDL_BUTTON_LEFT )
            {
                if ( event.type == SDL_MOUSEBUTTONUP )
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
                            else if ( activeButton != 0 ) { adjusted = RELEASED; playSfx( SELECT ); }
                            else if ( startActive ) { changeMenu = INGAME; scene = changeMenu; playSfx( SELECT ); }
                        case MULTI_MENU:
                            // gameHandler( false, activeButton );
                            // break;
                        case SETTINGS:
                            if ( backActive ) { changeMenu = MAIN_MENU; playSfx( SELECT ); }
                            else if ( activeButton != 0 ) { adjusted = RELEASED; playSfx( SELECT ); }
                            break;
                        case QUIT:
                            break;
                    }
                }
                else if ( event.type == SDL_MOUSEBUTTONDOWN )
                {
                    if ( scene == SETTINGS && activeButton != 0 ) adjusted = PRESSED;
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

int adjustmentSlider( int level, int x, int y, bool isHolding )
{
    int mouse_x, mouse_y;
    const int SLIDER_UNIT = LENGTH_UNIT / 10;
    SDL_GetMouseState( &mouse_x, &mouse_y );
    Texture slider;
    const string SLIDER_BG = "src/media/img/bg_slider.png";
    const string SLIDER_FG = "src/media/img/fg_slider.png";
    const string SLIDER_HEAD = "src/media/img/slider_head.png";
    slider.loadFromFile(SLIDER_BG);
    slider.render( x - LENGTH_UNIT * 5, y - LENGTH_UNIT / 4, LENGTH_UNIT * 10, LENGTH_UNIT / 2 );
    slider.loadFromFile(SLIDER_FG);
    slider.render( x - LENGTH_UNIT * 5, y - LENGTH_UNIT / 4, LENGTH_UNIT * level / 10, LENGTH_UNIT / 2 );
    slider.loadFromFile(SLIDER_HEAD);
    slider.render( x + LENGTH_UNIT * (level - 50) / 10 - LENGTH_UNIT / 2, y - LENGTH_UNIT / 2, LENGTH_UNIT, LENGTH_UNIT );
    int val = -1;
    if ( isHolding )
    {
        val = (mouse_x - ( x - LENGTH_UNIT * 5)) / SLIDER_UNIT;
        if ( val < 0 ) val = 0;
        else if ( val > 100 ) val = 100;
    }
    else if (mouse_x >= x - LENGTH_UNIT * 5 && mouse_x <= x + LENGTH_UNIT * 5 && mouse_y >= y - LENGTH_UNIT /2 && mouse_y <= y + LENGTH_UNIT / 2)
    {
        val = (mouse_x - ( x - LENGTH_UNIT * 5)) / SLIDER_UNIT;
    }
    return val;
}

void gameSettings( int &scene, int &activeButton, int &adjusted )
{
    enum category { RESOLUTION, BGM, SFX, PLAYFIELD_ELEMENT_SIZE, NEXT_BOX };
    int maxHeightOption = 5;
    for (int i = 0; i < 5; i++) if ( HEIGHT_ALLOWED[i + 1] > maxHeight ) {maxHeightOption = i; break;}
    if ( scene == SETTINGS )
    {
        enum settingPage { GENERAL, KEYBINDING };
        static int page = GENERAL;
        renderText( "SETTINGS" , LENGTH_UNIT * 6, LENGTH_UNIT * 4, false, LEFT, MIDDLE, 3, SDL_Color {255, 255, 255} );
        SDL_Rect rect { LENGTH_UNIT * 14, LENGTH_UNIT * 8, LENGTH_UNIT * 36, LENGTH_UNIT * 24 };
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
        SDL_RenderFillRect( renderer, &rect );

        switch(page)
        {
            case GENERAL:
            {
                static int activeSlider = -1;
                if (adjusted == RELEASED)
                {
                    activeSlider = -1;
                    adjusted = INITIAL;
                    switch( abs(activeButton) )
                    {
                        case RESOLUTION + 1:
                            for ( int i = 0; i < 6; i++ )
                            {
                                if ( HEIGHT_ALLOWED[i] == heightDimension ) {heightDimension = HEIGHT_ALLOWED[i + activeButton / abs(activeButton)]; break;}
                            }
                            applySettings( RESOLUTION );
                            break;
                        case SHOW_GHOST + 1:
                            showGhost += activeButton / abs(activeButton);
                            break;
                        case NEXT_BOXES + 1:
                            nextBoxes += activeButton / abs(activeButton);
                            break;
                    }
                    activeButton = 0;
                }
                int tmpSlider = -1, tmpButton = 0;
                renderText( "RESOLUTION" , LENGTH_UNIT * 16, LENGTH_UNIT * 12, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string(heightDimension * 16 / 9) + "x" + to_string(heightDimension), LENGTH_UNIT * 42, LENGTH_UNIT * 12, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpButton = adjustmentButton( LENGTH_UNIT * 42, LENGTH_UNIT * 12, heightDimension == HEIGHT_ALLOWED[0], heightDimension == HEIGHT_ALLOWED[maxHeightOption] ) * (RESOLUTION + 1);
                if ( tmpButton != 0 && activeSlider == -1 )
                {
                    activeButton = tmpButton;
                    tmpButton = 0;
                }

                renderText( "BACKGROUND MUSIC" , LENGTH_UNIT * 16, LENGTH_UNIT * 15, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( bgmVolume ), LENGTH_UNIT * 48, LENGTH_UNIT * 15, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpSlider = adjustmentSlider( bgmVolume, LENGTH_UNIT * 40, LENGTH_UNIT * 15, activeSlider == BGM_VOLUME && activeSlider != -1 );
                if ( tmpSlider != -1 && (activeSlider == -1 || activeSlider == BGM_VOLUME)  )
                {
                    activeButton = BGM_VOLUME + 1;
                    if ( adjusted == PRESSED )
                    {
                        bgmVolume = tmpSlider;
                        activeSlider = BGM_VOLUME;
                        applySettings(BGM_VOLUME);
                    }
                    tmpSlider = -1;
                }
                
                renderText( "SFX" , LENGTH_UNIT * 16, LENGTH_UNIT * 18, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( sfxVolume ), LENGTH_UNIT * 48, LENGTH_UNIT * 18, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpSlider = adjustmentSlider( sfxVolume, LENGTH_UNIT * 40, LENGTH_UNIT * 18, activeSlider == SFX_VOLUME && activeSlider != -1 );
                if ( tmpSlider != -1 && (activeSlider == -1 || activeSlider == SFX_VOLUME) )
                {
                    activeButton = SFX_VOLUME + 1;
                    if ( adjusted == PRESSED )
                    {
                        sfxVolume = tmpSlider;
                        activeSlider = SFX_VOLUME;
                        applySettings(SFX_VOLUME);
                    }
                    tmpSlider = -1;
                }

                renderText( "SCALE PLAYFIELD ELEMENTS" , LENGTH_UNIT * 16, LENGTH_UNIT * 21, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( (int)(playfieldScale * 100) ) + "%", LENGTH_UNIT * 48, LENGTH_UNIT * 21, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpSlider = adjustmentSlider( (playfieldScale - 0.5) / 0.0084, LENGTH_UNIT * 40, LENGTH_UNIT * 21, activeSlider == PLAYFIELD_SCALE && activeSlider != -1 );
                if ( tmpSlider != -1 && (activeSlider == -1 || activeSlider == PLAYFIELD_SCALE) )
                {
                    activeButton = PLAYFIELD_SCALE + 1;
                    if ( adjusted == PRESSED )
                    {
                        playfieldScale = 0.5 + 0.0084 * tmpSlider;
                        activeSlider = PLAYFIELD_SCALE;
                        applySettings(SFX_VOLUME);
                    }
                    tmpSlider = -1;
                }
                
                renderText( "SHOW GHOST" , LENGTH_UNIT * 16, LENGTH_UNIT * 24, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( (showGhost ? "ENABLED" : "DISABLED" ), LENGTH_UNIT * 42, LENGTH_UNIT * 24, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpButton = adjustmentButton( LENGTH_UNIT * 42, LENGTH_UNIT * 24, showGhost == 0, showGhost == 1 ) * (SHOW_GHOST + 1);
                if ( tmpButton != 0 && activeSlider == -1 )
                {
                    activeButton = tmpButton;
                    tmpButton = 0;
                }
                
                renderText( "NUMBER OF NEXT BOXES" , LENGTH_UNIT * 16, LENGTH_UNIT * 27, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( nextBoxes ), LENGTH_UNIT * 42, LENGTH_UNIT * 27, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpButton = adjustmentButton( LENGTH_UNIT * 42, LENGTH_UNIT * 27, nextBoxes == 1, nextBoxes == 5 ) * (NEXT_BOXES + 1);
                if ( tmpButton != 0 && activeSlider == -1 )
                {
                    activeButton = tmpButton;
                    tmpButton = 0;
                }
            }
            break;
            case KEYBINDING:
            {

            }
            break;
        }

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

void applySettings( int type )
{
    if ( type == RESOLUTION || type == -1 )
    {
        if ( WINDOW_HEIGHT != heightDimension )
        {
            WINDOW_HEIGHT = heightDimension;
            WINDOW_WIDTH = heightDimension * 16 / 9;
            if (game_window != NULL) SDL_SetWindowSize( game_window, WINDOW_WIDTH, WINDOW_HEIGHT );
        }
        LENGTH_UNIT = heightDimension / 36;
        changeDimensions();
        if ( type != -1 ) TTF_SetFontSize( fontBold, LENGTH_UNIT );
        if ( type != -1 ) TTF_SetFontSize( fontRegular, LENGTH_UNIT );
    }
    if ( type == BGM_VOLUME || type == -1 ) changeBgmVolume();
    if ( type == SFX_VOLUME || type == -1 ) changeSfxVolume();
    if ( type == PLAYFIELD_SCALE || type == RESOLUTION || type == -1 )
    {
        TILE_WIDTH = LENGTH_UNIT * playfieldScale;
        BOARD_HEIGHT = TILE_WIDTH * (HEIGHT_BY_TILE - HIDDEN_ROW);
        BOARD_WIDTH = TILE_WIDTH * WIDTH_BY_TILE;
    }
}