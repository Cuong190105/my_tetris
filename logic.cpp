#include <algorithm>
#include <iostream>
#include <sstream>
#include <random>
#include "logic.hpp"

using namespace std;

enum MBState { PRESSED = -1, INITIAL, RELEASED };

void generateTetromino( vector<Tetromino> &Tqueue, bool forceAdd )
{
    //7-bag randomization (Takes all 7 types of tetromino, shuffles them, then pushes them into the queue).
    //This method guarantees every unique pieces will be spawn within 7 turns, minimizes the missing/repetition
    //of a piece in some cases.
    if ( Tqueue.size() <= 7 || forceAdd ) {
        vector<int> v { I_PIECE, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE };
        random_device rd;
        mt19937 g(rd());
        shuffle(v.begin(), v.end(), g);
        for ( int i = 0; i < 7; i++ )
        {
            if (forceAdd) for (int j = 0; j < server.getClientNum(); j++)
            {
                server.makeMsg("T" + to_string(v[i]), j);
            }
            Tqueue.push_back( Tetromino( v[i] ) );
        }
        if (forceAdd) server.sendToClient();
    }
}

void handlePauseMenu( int &activeButton, int &mouse_x, int &mouse_y )
{
    renderText( "GAME PAUSED", WINDOW_WIDTH / 2, LENGTH_UNIT * 8, false, CENTER, MIDDLE, 2 );
    enum PauseMenuButton { CONTINUE, RETRY, SETTINGS, QUIT };
    SDL_Rect buttons[4];
    string content[] = { "CONTINUE", "RETRY", "SETTINGS", "BACK TO MENU" };
    buttons[CONTINUE] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 16, LENGTH_UNIT * 8, LENGTH_UNIT * 2};
    buttons[RETRY] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 19, LENGTH_UNIT * 8, LENGTH_UNIT * 2 };
    buttons[SETTINGS] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 22, LENGTH_UNIT * 8, LENGTH_UNIT * 2 };
    buttons[QUIT] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 25, LENGTH_UNIT * 8, LENGTH_UNIT * 2};
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
}

void processOpponentsControl( vector<Tetromino> &Tqueue, vector<int> &queuePosition, vector<Player> &player, vector<int>& recentEliminated, int &survivors )
{
    
    int minPos = queuePosition[0], maxPos = queuePosition[0];
    for ( int i = 1; i < queuePosition.size(); i++ )
    {
        if ( queuePosition[i] < minPos ) minPos = queuePosition[i];
        else if ( queuePosition[i] > maxPos ) maxPos = queuePosition[i];
    }
    
    if ( minPos >= 7 )
    {
        for ( int i = 0; i < queuePosition.size(); i++) queuePosition[i] -= 7;
        for ( int i = 0; i < 7; i++ ) Tqueue.erase( Tqueue.begin() );
    }
    
    if (isHost)
    {
        if ( Tqueue.size() - maxPos <= 7 )
        {
            generateTetromino( Tqueue, true );
        }
        
        server.receive();
        Sleep(5);
        
        vector<int> disconnectedList;
        for( int i = 0; i < server.getClientNum(); i++ )
        {
            string msg = server.getMsg( i );
            
            while ( msg.length() > 0 )
            {
                for ( int j = 0; j < server.getClientNum(); j++ )
                {
                    if ( j != i ) server.makeMsg( msg, j );
                }

                server.sendToClient();
                string cmd = msg.substr(1, 4);
                //The msg is actually "quit\x1F" but we slice the string from index 1 to 4
                if ( cmd == ("uit" + endMsg) || cmd == "term")
                {
                    if (cmd[0] == 'u') server.closeClientSocket( i );
                    player[msg[0]-'0'].terminateGame();
                    recentEliminated.push_back(i + 1 + (cmd[0] == 'u') * 100);
                    if ( cmd[0] == 't' )
                    {
                        for ( int i = 0; i < server.getClientNum(); i++ )
                        {
                            if ( i + 1 != msg[0] - '0' ) server.makeMsg( msg, i );
                        }
                        server.sendToClient();
                    }
                    break;
                }
                //gpul == gravity pull
                else if ( cmd == "gpul" ) player[msg[0] - '0'].gravityPull();
                //sdrp == soft drop
                else if ( cmd == "sdrp" ) player[msg[0] - '0'].dropPiece( false );
                //hdrp == hard drop
                else if ( cmd == "hdrp" ) player[msg[0] - '0'].dropPiece( true );
                //lmov = left move
                else if ( cmd == "lmov" ) player[msg[0] - '0'].movePieceHorizontally( false );
                //rmov = right move
                else if ( cmd == "rmov" ) player[msg[0] - '0'].movePieceHorizontally( true );
                //lrot = left(counterclockwise) rotate
                else if ( cmd == "lrot" ) player[msg[0] - '0'].rotatePiece( false );
                //rrot = right(clockwise) rotate
                else if ( cmd == "rrot" ) player[msg[0] - '0'].rotatePiece( true );
                //swap = swap hold piece
                else if ( cmd == "swap" ) player[msg[0] - '0'].swapHoldPiece();
                //pull = pull new piece
                else if ( cmd == "pull" ) player[msg[0] - '0'].pullNewTetromino( Tqueue, queuePosition[msg[0] - '0'] ); 
                //term = terminate
                if (msg[0] - '0' > i + 1) disconnectedList.push_back(i + 1);
                msg = server.getMsg( i );
            }
        }
        for (int i : disconnectedList)
        {
            recentEliminated.push_back(i + 100);
        }
        
    }
    else
    {
        client.receive();
        Sleep(5);
        if ( !client.isConnected() ) return;
        string msg = client.getMsg();
        while ( msg.length() > 0 || Tqueue.size() == 0 )
        {
            if ( msg[0] == 'T' )
            {
                Tqueue.push_back( Tetromino(msg[1] - '0') );
            }
            else if ( msg.length() >= 5 )
            {
                string cmd = msg.substr(1, 4);
                if ( cmd == "quit" )
                {
                    if (msg[0] == '0')
                    {
                        client.closeSocket();
                        playerList.clear();
                    }
                    else
                    {
                        if ( !player[msg[0] - '0'].isGameOver() )
                        {
                            player[msg[0] - '0'].terminateGame();
                            recentEliminated.push_back(msg[0] - '0' + 100);
                        }
                        // if ( msg[0] - '0' < client.getPosition() ) client.changePosition();
                    }
                }
                else if ( cmd == "gpul" ) player[msg[0] - '0'].gravityPull();
                else if ( cmd == "sdrp" ) player[msg[0] - '0'].dropPiece( false );
                else if ( cmd == "hdrp" ) player[msg[0] - '0'].dropPiece( true );
                else if ( cmd == "lmov" ) player[msg[0] - '0'].movePieceHorizontally( false );
                else if ( cmd == "rmov" ) player[msg[0] - '0'].movePieceHorizontally( true );
                else if ( cmd == "lrot" ) player[msg[0] - '0'].rotatePiece( false );
                else if ( cmd == "rrot" ) player[msg[0] - '0'].rotatePiece( true );
                else if ( cmd == "swap" ) player[msg[0] - '0'].swapHoldPiece();
                else if ( cmd == "pull" ) player[msg[0] - '0'].pullNewTetromino( Tqueue, queuePosition[msg[0] - '0'] );
                else if ( cmd == "term" ) {player[msg[0] - '0'].terminateGame();recentEliminated.push_back(msg[0] - '0');}
            }
            msg = client.getMsg();
        }
    }
}

void gameHandler( int players, int gameMode, int mod[4], int &scene, bool &transIn )
{
    bool play = true;
    Texture foreground;
    foreground.createTargetTexture();
    while ( play )
    {
        //Holds tetrominos for next turns
        vector<Tetromino> Tqueue;

        //Marks the start/pause/resume timestamps
        Uint32 startMark, pauseMark, resumeMark;

        //start/end flags
        bool start = false, win = false;
        enum pauseMenuOption { CONTINUE, RETRY, SETTINGS, QUIT_PLAY };

        //loads background
        loadRandomBackground();

        if ( players == 1 )
        {
            //Create a new player & modify game settings corresponding to chosen game mode
            Player player ( mod[LEVEL], gameMode, ( WINDOW_WIDTH - BOARD_WIDTH ) / 2, ( WINDOW_HEIGHT - BOARD_HEIGHT ) / 2 ) ;
            if ( gameMode == MASTER ) mod[LINECAP] = 300;
            else if ( gameMode == CLASSIC ) mod[LINECAP] = -1;
            else if ( gameMode == BLITZ ) mod[LINECAP] = 3;

            //Flag for polling tetromino from the queue
            int isDrawn = true;
            
            while ( !player.isGameOver() )
            {
                clearScreen();

                //Renders Bg Img
                bgImage.render();
                
                //Renders foreground elements (playfield, hold, next queue, etc) on a new texture for creating animation
                foreground.setAsTarget();
                player.displayBoard();
                player.displayTetrominoQueue( Tqueue );

                //Renders countdown
                if ( !start ) 
                {
                    SDL_Event catchQuit;
                    while ( SDL_PollEvent(&catchQuit) > 0 ) if ( catchQuit.type == SDL_QUIT ) { scene = QUIT; player.terminateGame(); play = false; }
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
                            playBackgroundMusic((gameMode == MASTER) ? FAST_THEME : CHILL_THEME);
                        }
                    }
                    SDL_SetRenderTarget( renderer, NULL );
                    foreground.render();
                }
                else if ( scene == INGAME )
                {
                    TTF_SetFontSize( fontBold, TILE_WIDTH * 3 / 4 );
                    TTF_SetFontSize( fontRegular, TILE_WIDTH * 3 / 4 );
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

                    //Handles win conditions, leveling && mystery events
                    switch( gameMode )
                    {
                        case SPRINT:
                            if ( player.getLine() >= mod[LINECAP] ) {player.terminateGame(); win = true;}
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
                            if ( player.getLevel() < 15 && player.getLine() / 10 + 1 > player.getLevel() ) player.setLevel( player.getLevel() + 1 );
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
                    if ( player.getMysteryEvent() == UPSIDE_DOWN ) foreground.render( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 180 );
                    else foreground.render();
                    if ( scene == PAUSE ) { pauseMark = SDL_GetTicks(); pauseMusic(); }
                    TTF_SetFontSize( fontBold, LENGTH_UNIT );
                    TTF_SetFontSize( fontRegular, LENGTH_UNIT );
                }
                else
                {
                    //Renders screen
                    TTF_SetFontSize( fontBold, TILE_WIDTH * 3 / 4 );
                    TTF_SetFontSize( fontRegular, TILE_WIDTH * 3 / 4 );
                    renderStatistics( player, startMark + SDL_GetTicks() - pauseMark, gameMode == TIME ? mod[TIME] : 0, mod[LINECAP] );
                    TTF_SetFontSize( fontBold, LENGTH_UNIT );
                    TTF_SetFontSize( fontRegular, LENGTH_UNIT );
                    player.displayHeldTetromino();
                    player.displayBoard();
                    player.displayCurrentTetromino();
                    SDL_SetRenderTarget( renderer, NULL );
                    if ( player.getMysteryEvent() == UPSIDE_DOWN ) foreground.render( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 180 );
                    else foreground.render();
                    // if ( gameMode == MYSTERY && player.getMysteryEvent() == UPSIDE_DOWN ) SDL_RenderCopyEx( renderer, foreground, NULL, NULL, 180, NULL, SDL_FLIP_NONE );
                    // else SDL_RenderCopy( renderer, foreground, NULL, NULL );
                    SDL_Rect overlay { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
                    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
                    SDL_RenderFillRect( renderer, &overlay );

                    //Handles events
                    SDL_Event e;
                    static int activeButton, adjusted;
                    int mouse_x, mouse_y;
                    SDL_GetMouseState( &mouse_x, &mouse_y );
                    if ( scene == PAUSE ) handlePauseMenu( activeButton, mouse_x, mouse_y );
                    else if ( scene == INGAME_SETTINGS )
                    {
                        gameSettings( scene, activeButton, adjusted, mouse_x, mouse_y );
                        if ( handleBackButton( mouse_x, mouse_y ) && activeButton == 0 )
                        {
                            activeButton = 100;
                        }
                        player.setX( (WINDOW_WIDTH - BOARD_WIDTH) / 2 );
                        player.setY( (WINDOW_HEIGHT - BOARD_HEIGHT) / 2 );
                    }
                    while ( SDL_PollEvent( &e ) )
                    {
                        if ( e.type == SDL_QUIT )
                        {
                            play = false;
                            player.terminateGame();
                            scene = QUIT;
                        }
                        else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE ) scene = (scene == PAUSE ? INGAME : PAUSE);
                        else if ( activeButton != -1 && e.button.button == SDL_BUTTON_LEFT )
                        {
                            if (  e.type == SDL_MOUSEBUTTONUP )
                            {
                                if ( scene == PAUSE )
                                {
                                    switch(activeButton)
                                    {
                                        case CONTINUE:
                                            scene = INGAME;
                                            activeButton = -1;
                                            break;
                                        case RETRY:
                                            player.terminateGame();
                                            start = false;
                                            stopMusic( false );
                                            scene = INGAME;
                                            activeButton = -1;
                                            break;
                                        case SETTINGS:
                                            scene = INGAME_SETTINGS;
                                            activeButton = -1;
                                            break;
                                        case QUIT_PLAY:
                                            play = false;
                                            player.terminateGame();
                                            scene = SOLO_MENU;
                                            break;
                                    }
                                    playSfx( SELECT );
                                }
                                else if ( scene == INGAME_SETTINGS )
                                {
                                    // if ( adjusted == PRESSED || activeButton != 0 ) playSfx( SELECT );
                                    adjusted = INITIAL;
                                    if ( activeButton == 100 )
                                    {
                                        scene = PAUSE;
                                        activeButton = -1;
                                    }
                                }
                            }
                            else if ( scene == INGAME_SETTINGS && e.type == SDL_MOUSEBUTTONDOWN )
                            {
                                if ( activeButton != -1 ) {adjusted = PRESSED; playSfx( SELECT );}
                            }
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
                updateHighScore( gameMode, player.getScore(), player.getLine(), int(endMark - startMark) );
                saveHighScore();
                while ( SDL_GetTicks() - endMark <= 5000)
                {
                    
                    clearScreen();
                    bgImage.render();
                    TTF_SetFontSize( fontBold, TILE_WIDTH * 3 / 4 );
                    TTF_SetFontSize( fontRegular, TILE_WIDTH * 3 / 4 );
                    foreground.setAsTarget();
                    
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
                    foreground.render();
                    SDL_RenderPresent( renderer );
                    
                }
                
                TTF_SetFontSize( fontBold, LENGTH_UNIT );
                TTF_SetFontSize( fontRegular, LENGTH_UNIT );

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
                    foreground.setAlphaMod( 255 * max((int)( 500 - SDL_GetTicks() + endMark ), 0) / 500 );
                    foreground.render();
                    SDL_RenderPresent( renderer );
                    
                }

                endMark = SDL_GetTicks();
                while ( SDL_GetTicks() - endMark <= 6500 )
                {
                    clearScreen();
                    bgImage.render();
                    foreground.setAsTarget();
                    renderResultScreen(player, endMark, time );
                    SDL_SetRenderTarget( renderer, NULL );
                    foreground.render();
                    if (SDL_GetTicks() - endMark <= 500)
                    {
                        foreground.setAlphaMod( min(255 * (int)(SDL_GetTicks() - endMark) / 500, 255) );
                    }
                    else if ( SDL_GetTicks() - endMark > 6000 )
                    {
                        foreground.setAlphaMod( max(255 * (int)(6500 - SDL_GetTicks() + endMark) / 500, 0) );
                    }
                    foreground.render();
                    SDL_RenderPresent( renderer );
                    
                }

            }
        }

        //Handles multiplayer modes
        else
        {
            int mainPos = isHost ? 0 : client.getPosition();
            
            int survivors = playerList.size();
            
            vector<int> survivorList;

            vector<Player> player;

            for (int i = 0; i < survivors; i++)
            {
                int x = (WINDOW_WIDTH - BOARD_WIDTH) / 2, y = (WINDOW_HEIGHT - BOARD_HEIGHT) / 2;
                if (i == mainPos) player.push_back( Player(mod[LEVEL], gameMode, x, y, true));
                else player.push_back( Player(mod[LEVEL], gameMode, x, y, false));
            }
            vector<int> queuePosition( survivors, 0 );

            //Target textures for each playfield
            Texture mainPlayerTexture;
            mainPlayerTexture.createTargetTexture();
            vector<Texture> sidePlayerTextures ( survivors);
            for ( int i = 0; i < sidePlayerTextures.size(); i++ )
            {
                sidePlayerTextures[i].createTargetTexture();
            }

            //Stores match results & essential flags for starting/terminating match or doing animation
            vector<int> winCounter (survivors, 0);
            bool endMatch = false, startRound = false, anyEliminated = true;
            bool changeLayout = false;
            vector<Uint32> eliminationMark (survivors);
            Uint32 lastEliMark = SDL_GetTicks();

            //The frame that trims the source texture used in SDL_RenderCopy
            SDL_Rect TRIM_FRAME { WINDOW_WIDTH / 4, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT };

            /**
             * Sample frames as layout for displaying playfields. The used frames will be set to sample at the start of the game
             * and will gradually change & distribute to match the new sample if any player is eliminated to create the animation.
             */

            vector<SDL_Rect> sampleSideFrame (survivors, {0, 0, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2});
            
            vector<SDL_Rect> tmpFrame (survivors, {0, 0, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2});

            //Stores recently eliminated players
            vector<int> recentEliminated;

            //Stores disconnected players.
            vector<int> disconnected;

            for (int i = 0; i < playerList.size(); i++) survivorList.push_back(i);
            //Main frame for this player, side frames for the opponents
            SDL_Rect mainFrame { 0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT };
            SDL_Rect sampleMainFrame { 0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT };

            vector<SDL_Rect> sideFrame ( survivors );
            
            const int animationDuration = 1000; 
            
            while ( !endMatch )
            {
                thread handleOpponent;
                if ( startRound )
                {
                    processOpponentsControl( Tqueue, queuePosition, player, recentEliminated, survivors );
                    // handleOpponent = thread(processOpponentsControl, ref(Tqueue), ref(queuePosition), ref(player), ref(recentEliminated), ref(survivors));
                }

                //Handle quit events
                SDL_Event e;
                while( SDL_PollEvent(&e) > 0 )
                {
                    if ( e.type == SDL_QUIT )
                    {
                        isHost ? server.closeServer() : client.closeSocket();
                        endMatch = true;
                        play = false;
                        scene = QUIT;
                    }
                }

                //Cancel game if the client is disconnected or the server does not have any client.
                if ( (!isHost && !client.isConnected()) || (isHost && server.getClientNum() == 0) )
                {
                    if ( handleOpponent.joinable() ) handleOpponent.join();
                    scene = MULTI_MENU;
                    play = false;
                    break;
                }

                //Managing game logical process
                
                if (startRound && !player[mainPos].isGameOver()) 
                {
                    player[mainPos].ingameProgress( Tqueue, queuePosition[mainPos], scene );
                    if ( player[mainPos].isGameOver() )
                    {
                        recentEliminated.push_back( mainPos );
                    }
                }
                
                while ( !recentEliminated.empty() )
                {
                    if ( recentEliminated[0] >= 100 )
                    {
                        recentEliminated[0] -= 100;
                        disconnected.push_back(recentEliminated[0]);
                    }
                    for (int i = 0; i < survivors; i++)
                    {
                        cout << "pop pl " << recentEliminated[0];
                        if (i == recentEliminated[0]) survivorList.erase( survivorList.begin() + i ); break;
                    }
                    eliminationMark.push_back( SDL_GetTicks() );
                    lastEliMark = SDL_GetTicks();
                    survivors--;                
                    anyEliminated = true;
                    recentEliminated.erase( recentEliminated.begin() );
                }
                
                //Managing render process
                //Renders to each frame
                
                clearScreen();
                
                for ( int i = 0; i < survivors; i++ )
                {
                    if ( survivorList[i] == mainPos ) mainPlayerTexture.setAsTarget();
                    else sidePlayerTextures[survivorList[i] ].setAsTarget();
                    player[survivorList[i]].displayBoard();
                    
                    for ( int j = 0; j < playerList.size(); j++)
                    {
                        if ( j == survivorList[i] )
                        {
                            renderText( playerList[j].name, WINDOW_WIDTH / 2, player[survivorList[i]].getY() + BOARD_HEIGHT + TILE_WIDTH, false, CENTER, MIDDLE );
                            renderText( to_string(winCounter[j]) + " Win" + (winCounter[survivorList[i]] > 1 ? "s" : ""), WINDOW_WIDTH / 2, player[survivorList[i]].getY() + BOARD_HEIGHT + TILE_WIDTH * 2, false, CENTER, MIDDLE );
                            break;
                        }
                        
                    }
                    if ( !startRound )
                    {
                        if ( !transIn )
                        {
                            startRound = displayCountdown(player[survivorList[i]].getX(), player[survivorList[i]].getY(), BOARD_WIDTH, BOARD_HEIGHT, startMark) && survivorList[i] == survivors - 1;
                            if ( startRound && survivorList[i] == survivors - 1 )
                            {
                                startMark = SDL_GetTicks();
                                playBackgroundMusic( FAST_THEME );
                            }
                        }
                    }
                    else if ( scene == INGAME )
                    {
                        TTF_SetFontSize( fontBold, TILE_WIDTH * 3 / 4 );
                        TTF_SetFontSize( fontRegular, TILE_WIDTH * 3 / 4 );
                        player[survivorList[i]].displayTetrominoQueue( Tqueue, queuePosition[survivorList[i]] );
                        renderStatistics( player[survivorList[i]], startMark );
                        player[survivorList[i]].displayCurrentTetromino();
                        player[survivorList[i]].displayHeldTetromino();
                        player[survivorList[i]].displayBonus();
                        TTF_SetFontSize( fontBold, LENGTH_UNIT );
                        TTF_SetFontSize( fontRegular, LENGTH_UNIT );
                    }
                }
                SDL_SetRenderTarget( renderer, NULL );

                //Calculates the sample render frames at the start of the game or when any player is eliminated
                if ( anyEliminated )
                {
                    vector<int> sidePlayer;
                    for (int i = 0; i < survivors; i++) if (survivorList[i] != mainPos) sidePlayer.push_back(survivorList[i]);
                    if ( !player[mainPos].isGameOver() )
                    {
                        if ( survivors == 1)
                        {
                            sampleMainFrame.w *= 2;
                        }
                        else if ( survivors > 1 )
                        {
                            sampleSideFrame[survivorList[sidePlayer[0]]].x = WINDOW_WIDTH / 2;
                            if (survivors == 2)
                            {
                                sampleSideFrame[survivorList[sidePlayer[0]]].w *= 2;
                                sampleSideFrame[survivorList[sidePlayer[0]]].h *= 2;
                            }
                            else
                            {
                                sampleSideFrame[survivorList[sidePlayer[1]]].x = WINDOW_WIDTH * 3 / 4;
                                if (survivors == 3)
                                {
                                    sampleSideFrame[survivorList[sidePlayer[0]]].y = WINDOW_HEIGHT / 4;
                                    sampleSideFrame[survivorList[sidePlayer[1]]].y = WINDOW_HEIGHT / 4;
                                }
                                else
                                {
                                    sampleSideFrame[survivorList[sidePlayer[2]]].x = WINDOW_WIDTH * 5 / 8;
                                    sampleSideFrame[survivorList[sidePlayer[2]]].y = WINDOW_HEIGHT / 2;
                                }
                            }
                        }
                    }
                    else
                    {
                        for ( int i = 0; i < survivors; i++ )
                        {
                            sampleSideFrame[survivorList[sidePlayer[i]] ].w = WINDOW_WIDTH / survivors;
                            sampleSideFrame[survivorList[sidePlayer[i]] ].h = 2 * WINDOW_HEIGHT / survivors;
                            sampleSideFrame[survivorList[sidePlayer[i]] ].x = i * sampleSideFrame[survivorList[sidePlayer[i]] ].w;
                            sampleSideFrame[survivorList[sidePlayer[i]] ].y = (WINDOW_HEIGHT - sampleSideFrame[survivorList[sidePlayer[i]] ].h) / 2;
                        }
                    }
                    
                    if (changeLayout)
                    {
                        for (int i = 0; i < survivors; i++) if (survivorList[i] != mainPos)
                        {
                            sideFrame[survivorList[i] ] = tmpFrame[survivorList[i] ]; 
                        }
                    }

                    anyEliminated = false;
                    if (startRound) changeLayout = true;
                    //Assigns values for the used frames at the start of the game
                    if ( !startRound )
                    {
                        for ( int i = 0; i < sideFrame.size(); i++ )
                                sideFrame[i] = sampleSideFrame[i];
                    }
                }

                //Renders each frame to the main window
                bgImage.render();

                if ( changeLayout)
                {
                    //get delta time
                    Uint32 now = SDL_GetTicks();
                    double progress = (now - lastEliMark) * 1.f / animationDuration;

                    if ( progress < 1 )
                    {
                        //Quadratic ease out
                        double relativePosition = 1 - (1 - progress) * (1 - progress);

                        if ( survivors == 1 )
                        {
                            TRIM_FRAME.w = (WINDOW_WIDTH / 2) * relativePosition + WINDOW_WIDTH / 2;
                            TRIM_FRAME.x = (WINDOW_WIDTH - TRIM_FRAME.w) / 2;
                        }

                        for (int i = 0; i < survivors; i++) if ( survivorList[i] != mainPos )
                        {
                            tmpFrame[survivorList[i] ] = {
                                (int)((sampleSideFrame[survivorList[i] ].x - sideFrame[survivorList[i] ].x) * relativePosition + sideFrame[survivorList[i] ].x),
                                (int)((sampleSideFrame[survivorList[i] ].y - sideFrame[survivorList[i] ].y) * relativePosition + sideFrame[survivorList[i] ].y),
                                (int)((sampleSideFrame[survivorList[i] ].w - sideFrame[survivorList[i] ].w) * relativePosition + sideFrame[survivorList[i] ].w),
                                (int)((sampleSideFrame[survivorList[i] ].h - sideFrame[survivorList[i] ].h) * relativePosition + sideFrame[survivorList[i] ].h),
                            };
                            sidePlayerTextures[i].render( tmpFrame[survivorList[i] ].x, tmpFrame[survivorList[i] ].y, tmpFrame[survivorList[i] ].w, tmpFrame[survivorList[i] ].h, &TRIM_FRAME);
                        }

                        if ( survivors == 1 && survivorList[0] == mainPos )
                        {
                            SDL_Rect tmpFrame = {
                                (int)((sampleMainFrame.x - mainFrame.x) * relativePosition + mainFrame.x),
                                (int)((sampleMainFrame.y - mainFrame.y) * relativePosition + mainFrame.y),
                                (int)((sampleMainFrame.w - mainFrame.w) * relativePosition + mainFrame.w),
                                (int)((sampleMainFrame.h - mainFrame.h) * relativePosition + mainFrame.h),
                            };
                            mainPlayerTexture.render( tmpFrame.x, tmpFrame.y, tmpFrame.w, tmpFrame.h, &TRIM_FRAME );
                        }
                    }
                    else
                    {
                        for (int i = 0; i < survivors; i++) if ( survivorList[i] != mainPos )
                        {
                            sideFrame[survivorList[i] ] = sampleSideFrame[survivorList[i] ];
                            sidePlayerTextures[survivorList[i] ].render( sideFrame[survivorList[i] ].x, sideFrame[survivorList[i] ].x, sideFrame[survivorList[i] ].w, sideFrame[survivorList[i] ].h, &TRIM_FRAME);
                        }
                    }

                    //Render fading board of lost player
                    for ( int i = 0; i < playerList.size(); i++ )
                    {
                        if ( now - eliminationMark[i] < animationDuration )
                        {
                            if ( i == mainPos )
                            {
                                mainPlayerTexture.setAlphaMod( 255 * ( 1 - (now - eliminationMark[i]) * 1.f / animationDuration ) );
                                mainPlayerTexture.render( mainFrame.x, mainFrame.y, mainFrame.w, mainFrame.h, &TRIM_FRAME );
                            }
                            else 
                            {
                                sidePlayerTextures[i].setAlphaMod( 255 * ( 1 - (now - eliminationMark[i]) * 1.f / animationDuration ) );
                                sidePlayerTextures[i].render( sideFrame[i].x, sideFrame[i].y, sideFrame[i].w, sideFrame[i].h, &TRIM_FRAME );    
                            }
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < survivors; i++) if ( survivorList[i] != mainPos )
                    {
                        sidePlayerTextures[survivorList[i] ].render( sideFrame[survivorList[i] ].x, sideFrame[survivorList[i] ].y, sideFrame[survivorList[i] ].w, sideFrame[survivorList[i] ].h, &TRIM_FRAME);
                    }
                }


                if (!player[mainPos].isGameOver()) mainPlayerTexture.render( mainFrame.x, mainFrame.y, mainFrame.w, mainFrame.h, &TRIM_FRAME );
                

                //Render Transitions overlay
                if ( !startRound && transIn )
                {
                    renderTransition( transIn );
                    if ( !transIn )
                    {
                        if (isHost)
                        {
                            int synced = 0;
                            for ( int i = 0; i < server.getClientNum(); i++ ) server.makeMsg( "sync", i );
                            server.sendToClient();
                            generateTetromino( Tqueue, true );
                            while ( synced < server.getClientNum() )
                            {
                                server.receive();
                                for ( int i = 0; i < server.getClientNum(); i++ )
                                {
                                    if ( server.getMsg(i) == "ok" ) synced++;
                                }
                            }
                            for ( int i = 0; i < server.getClientNum(); i++ ) server.makeMsg( "start", i );
                            server.sendToClient();
                        }
                        else
                        {
                            string msg = "";
                            do
                            {
                                client.receive();
                                msg = client.getMsg();
                            } while ( msg != "sync" );
                            Sleep(10);
                            processOpponentsControl( Tqueue, queuePosition, player, recentEliminated, survivors );
                            client.sendToServer( "ok" );
                            do
                            {
                                client.receive();
                                msg = client.getMsg();
                            } while ( msg != "start" );
                        }
                        startMark = SDL_GetTicks();
                    }
                }

                //Display render buffer
                SDL_RenderPresent( renderer );
                if (handleOpponent.joinable()) handleOpponent.join();
                
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
                foreground.setAsTarget();
                clearScreen();
                int activeButton = renderRetryScreen( retryLoop, scene );
                SDL_SetRenderTarget( renderer, NULL );
                if (SDL_GetTicks() - mark < 500)
                {
                    foreground.setAlphaMod( 255 * (SDL_GetTicks() - mark) / 500 );
                }
                foreground.render();
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
            if (players == 1) foreground.render();
            renderTransition( transIn );
            SDL_RenderPresent( renderer );
        }
    }
    if (gameMode >= SCORE)
    {
        if (isHost) server.closeServer();
        else client.closeSocket();
    }
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

void settingRules( int gameMode, int &activeButton, int &adjusted, int mod[4] )
{
    SDL_Rect rect { LENGTH_UNIT * 12, LENGTH_UNIT * 6, LENGTH_UNIT * 40, LENGTH_UNIT * 24 };
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
    SDL_RenderFillRect( renderer, &rect);
    int mouse_x, mouse_y;
    renderText( gameModeName[gameMode], WINDOW_WIDTH / 2, LENGTH_UNIT * 12, true, CENTER, BOTTOM, 4, SDL_Color {255, 255, 255} );
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

void menuManager( int &scene, bool &transIn, int &players,  int &gameMode, int mod[4]  )
{
    loadMenuElements();
    Texture title;
    title.loadFromFile(GAME_TITLE);
    int activeButton = -1;
    int mouse_x, mouse_y;
    SDL_Event event;
    int foregroundAlphaMod = 10;
    vector<Tetromino> floating;
    int adjusted = 0;
    int changeMenu = scene;
    SDL_Keycode key = 0;
    string text = "";
    Uint32 animationMark = SDL_GetTicks();
    const int ANIMATION_DURATION = 500;
    while ( scene != INGAME && scene != QUIT )
    {
        SDL_GetMouseState( &mouse_x, &mouse_y );
        bool backActive = false, startActive = false;

        //Renders Screen & Gets button states ( anyActive = Active )
        clearScreen();
        renderMenuBackground();
        generateTetromino( floating );
        renderFloatingTetromino( floating );

        SDL_Texture *foreground = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT );
        SDL_SetRenderTarget( renderer, foreground );
        SDL_SetTextureBlendMode( foreground, SDL_BLENDMODE_BLEND );
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
        SDL_RenderClear( renderer );
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
                settingRules( gameMode, activeButton, adjusted, mod );
                startActive = handleStartButton( mouse_x, mouse_y, WINDOW_WIDTH / 2, LENGTH_UNIT * 26 );
                backActive = handleBackButton( mouse_x, mouse_y );
                break;

            case MAIN_MENU_SETTINGS:
                backActive = handleBackButton( mouse_x, mouse_y );
                gameSettings( scene, activeButton, adjusted, mouse_x, mouse_y );
                break;
            case MULTI_MENU:
            case CREATE_SERVER:
            case JOIN_SERVER:
            case MULTI_LOBBY:
                // renderText( "Under Construction", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, true, CENTER, MIDDLE, 3, SDL_Color {255, 255, 255} );
                multiplayerManager( scene, changeMenu, mouse_x, mouse_y, activeButton, adjusted, key, text );
                key = 0;
                text = "";
                backActive = handleBackButton( mouse_x, mouse_y );
                adjusted = false;
                if ( changeMenu == INGAME )
                {
                    players = playerList.size();
                    gameMode = mInfo.gameMode;
                    mod[LEVEL] = mInfo.lvlSpd;
                }
                break;
            default:
                scene = QUIT;
        }
        SDL_SetRenderTarget( renderer, NULL );

        //Handles mouse events
        while ( SDL_PollEvent( &event ) > 0 )
        {
            if ( event.type == SDL_QUIT )
            { 
                if ( scene == MULTI_LOBBY )
                {
                    isHost ? server.closeServer() : client.closeSocket();
                }
                scene = QUIT; break;
            }
            else if ( changeMenu != scene ) continue;
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
                        case CREATE_SERVER:
                        case JOIN_SERVER:
                            if ( backActive ) {changeMenu = (scene != MULTI_MENU) ? MULTI_MENU : MAIN_MENU; playSfx( SELECT );}
                            else if ( activeButton > -1 )
                            {
                                adjusted = 1;
                                playSfx( SELECT );
                            }
                            break;
                        case MULTI_LOBBY:
                            if ( backActive )
                            {
                                isHost ? server.closeServer() : client.closeSocket();
                                playerList.clear();
                                changeMenu = MULTI_MENU;
                                playSfx( SELECT );
                            }
                            else if ( activeButton != 0 )
                            {
                                adjusted = true;
                                playSfx( SELECT );
                            }
                            break;
                        case MAIN_MENU_SETTINGS:
                            if ( backActive ) { changeMenu = MAIN_MENU; adjusted = INITIAL; activeButton = 0; playSfx( SELECT ); }
                            else if ( activeButton != 0 ) { adjusted = RELEASED; playSfx( SELECT ); }
                            break;
                        case QUIT:
                            break;
                        
                    }
                }
                else if ( event.type == SDL_MOUSEBUTTONDOWN )
                {
                    if ( scene == MAIN_MENU_SETTINGS && activeButton != 0 ) adjusted = PRESSED;
                }
            }
            else if ( event.type == SDL_TEXTINPUT )
            {
                text = event.text.text;
            }
            else if ( event.type == SDL_KEYDOWN )
            {
                key = event.key.keysym.sym;
            }
        }

        //Plays transition if necessary
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
                        adjusted = INITIAL;
                        activeButton = 0;
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

void gameSettings( int &scene, int &activeButton, int &adjusted, int mouse_x, int mouse_y )
{
    enum category { RESOLUTION, BGM, SFX, PLAYFIELD_ELEMENT_SIZE, NEXT_BOX };
    int maxHeightOption = 5;
    for (int i = 0; i < 5; i++) if ( HEIGHT_ALLOWED[i + 1] > maxHeight ) {maxHeightOption = i; break;}
    bool anyActive = false;
    static int activeSlider = -1;
    if ( scene == MAIN_MENU_SETTINGS )
    {
        enum settingPage { GENERAL = 100, CONTROL };
        static int page = GENERAL;
        renderText( "SETTINGS" , LENGTH_UNIT * 6, LENGTH_UNIT * 4, true, LEFT, MIDDLE, 3, SDL_Color {255, 255, 255} );
        SDL_Rect rect { LENGTH_UNIT * 14, LENGTH_UNIT * 8, LENGTH_UNIT * 36, LENGTH_UNIT * 24 };
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 225 );
        SDL_RenderFillRect( renderer, &rect );

        if (  mouse_y >= LENGTH_UNIT * 9 && mouse_y <= LENGTH_UNIT * 11 )
        {
            if ( mouse_x >= LENGTH_UNIT * 23 && mouse_x <= LENGTH_UNIT * 29 ) {activeButton = GENERAL; anyActive = true;}
            else if ( mouse_x >= LENGTH_UNIT * 35 && mouse_x <= LENGTH_UNIT * 41 ) {activeButton = CONTROL; anyActive = true;}
        }

        if ( adjusted == RELEASED && anyActive == true )
        {
            page = activeButton;
            activeButton = 0;
            adjusted = INITIAL;
        }

        switch(page)
        {
            case GENERAL:
            {
                renderText( "GENERAL", LENGTH_UNIT * 26, LENGTH_UNIT * 10, true, CENTER, MIDDLE, 1.5 );
                renderText( "CONTROL", LENGTH_UNIT * 38, LENGTH_UNIT * 10, true, CENTER, MIDDLE, 1.5, SDL_Color {50, 50, 50} );
                Texture underlineBar;
                underlineBar.loadFromFile(SLIDER_FG);
                int w, h;
                TTF_SizeText( fontBold, "GENERAL", &w, &h );
                int x = LENGTH_UNIT * 26 - w / 2, y = LENGTH_UNIT * 10 + h;
                h = LENGTH_UNIT / 5;
                underlineBar.render( x, y, w, h );

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
                renderText( "RESOLUTION" , LENGTH_UNIT * 16, LENGTH_UNIT * 14, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string(heightDimension * 16 / 9) + "x" + to_string(heightDimension), LENGTH_UNIT * 42, LENGTH_UNIT * 14, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpButton = adjustmentButton( LENGTH_UNIT * 42, LENGTH_UNIT * 14, heightDimension == HEIGHT_ALLOWED[0], heightDimension == HEIGHT_ALLOWED[maxHeightOption] ) * (RESOLUTION + 1);
                if ( tmpButton != 0 && activeSlider == -1 )
                {
                    activeButton = tmpButton;
                    tmpButton = 0;
                    anyActive = true;
                }

                renderText( "BACKGROUND MUSIC" , LENGTH_UNIT * 16, LENGTH_UNIT * 17, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( bgmVolume ), LENGTH_UNIT * 48, LENGTH_UNIT * 17, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpSlider = adjustmentSlider( bgmVolume, LENGTH_UNIT * 40, LENGTH_UNIT * 17, activeSlider == BGM_VOLUME && activeSlider != -1 );
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
                    anyActive = true;
                }
                
                renderText( "SFX" , LENGTH_UNIT * 16, LENGTH_UNIT * 20, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( sfxVolume ), LENGTH_UNIT * 48, LENGTH_UNIT * 20, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpSlider = adjustmentSlider( sfxVolume, LENGTH_UNIT * 40, LENGTH_UNIT * 20, activeSlider == SFX_VOLUME && activeSlider != -1 );
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
                    anyActive = true;
                }

                renderText( "SCALE PLAYFIELD ELEMENTS" , LENGTH_UNIT * 16, LENGTH_UNIT * 23, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( (int)(playfieldScale * 100) ) + "%", LENGTH_UNIT * 48, LENGTH_UNIT * 23, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpSlider = adjustmentSlider( (playfieldScale - 0.5) / 0.0084, LENGTH_UNIT * 40, LENGTH_UNIT * 23, activeSlider == PLAYFIELD_SCALE && activeSlider != -1 );
                if ( tmpSlider != -1 && (activeSlider == -1 || activeSlider == PLAYFIELD_SCALE) )
                {
                    activeButton = PLAYFIELD_SCALE + 1;
                    if ( adjusted == PRESSED )
                    {
                        playfieldScale = 0.5 + 0.0084 * tmpSlider;
                        activeSlider = PLAYFIELD_SCALE;
                        applySettings(PLAYFIELD_SCALE);
                    }
                    tmpSlider = -1;
                    anyActive = true;
                }
                
                renderText( "SHOW GHOST" , LENGTH_UNIT * 16, LENGTH_UNIT * 26, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( (showGhost ? "ENABLED" : "DISABLED" ), LENGTH_UNIT * 42, LENGTH_UNIT * 26, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpButton = adjustmentButton( LENGTH_UNIT * 42, LENGTH_UNIT * 26, showGhost == 0, showGhost == 1 ) * (SHOW_GHOST + 1);
                if ( tmpButton != 0 && activeSlider == -1 )
                {
                    activeButton = tmpButton;
                    tmpButton = 0;
                    anyActive = true;
                }
                
                renderText( "NUMBER OF NEXT BOXES" , LENGTH_UNIT * 16, LENGTH_UNIT * 29, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
                renderText( to_string( nextBoxes ), LENGTH_UNIT * 42, LENGTH_UNIT * 29, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                tmpButton = adjustmentButton( LENGTH_UNIT * 42, LENGTH_UNIT * 29, nextBoxes == 1, nextBoxes == 5 ) * (NEXT_BOXES + 1);
                if ( tmpButton != 0 && activeSlider == -1 )
                {
                    activeButton = tmpButton;
                    tmpButton = 0;
                    anyActive = true;
                }
                break;
            }
            case CONTROL:
            {
                bool keybindScreen = false;

                renderText( "CONTROL", LENGTH_UNIT * 38, LENGTH_UNIT * 10, true, CENTER, MIDDLE, 1.5 );
                renderText( "GENERAL", LENGTH_UNIT * 26, LENGTH_UNIT * 10, true, CENTER, MIDDLE, 1.5, SDL_Color {50, 50, 50} );
                Texture underlineBar;
                underlineBar.loadFromFile(SLIDER_FG);
                int w, h;
                TTF_SizeText( fontBold, "CONTROL", &w, &h );
                int x = LENGTH_UNIT * 38 - w / 2, y = LENGTH_UNIT * 10 + h;
                h = LENGTH_UNIT / 5;
                underlineBar.render( x, y, w, h );

                renderText( "MOVE LEFT" , LENGTH_UNIT * 16, LENGTH_UNIT * 14, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );                
                renderText( "MOVE RIGHT" , LENGTH_UNIT * 16, LENGTH_UNIT * 16, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );                
                renderText( "SOFT DROP" , LENGTH_UNIT * 16, LENGTH_UNIT * 18, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );                
                renderText( "HARD DROP" , LENGTH_UNIT * 16, LENGTH_UNIT * 20, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );                
                renderText( "SWAP HOLD" , LENGTH_UNIT * 16, LENGTH_UNIT * 22, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );                
                renderText( "ROTATE CLOCKWISE" , LENGTH_UNIT * 16, LENGTH_UNIT * 24, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );                
                renderText( "ROTATE COUNTERCLOCKWISE" , LENGTH_UNIT * 16, LENGTH_UNIT * 26, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );

                int mouse_x, mouse_y;
                SDL_GetMouseState( &mouse_x, &mouse_y );
                for (int i = 0; i < NUM_KEY_FUNCTIONS; i++)
                {
                    if (handleKeybindButton(keyScanCode[i] != 0 ? string(SDL_GetKeyName( SDL_GetKeyFromScancode( keyScanCode[i]) )) : "<->",
                                            mouse_x, mouse_y, LENGTH_UNIT * (40 + 4 * ( i & 1 ) ), LENGTH_UNIT * (14 + i / 2 * 2),
                                            LENGTH_UNIT * 3, LENGTH_UNIT, {150, 150, 150} ))
                    {
                        activeButton = i + 1;
                        anyActive = true;
                    }
                }
                if ( adjusted == RELEASED && activeButton > 0 )
                {
                    adjusted = INITIAL;
                    keybindScreen = true;
                }
                while ( keybindScreen == true )
                {
                    SDL_SetRenderTarget( renderer, NULL );
                    clearScreen();
                    SDL_Rect background { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
                    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
                    SDL_RenderFillRect( renderer, &background );
                    renderText( "PRESS A KEY TO BIND", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, true, CENTER, MIDDLE, 4 );
                    renderText( "CLICK ANYWHERE TO REMOVE KEYBIND", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + LENGTH_UNIT * 4, true, CENTER, MIDDLE, 2 );
                    SDL_RenderPresent( renderer );

                    SDL_Event catchKey;
                    while (SDL_PollEvent(&catchKey))
                    {
                        if ( catchKey.type == SDL_QUIT ) { scene = QUIT; keybindScreen = false; break; }
                        else if ( catchKey.type == SDL_KEYDOWN)
                        {
                            keyScanCode[activeButton - 1] = catchKey.key.keysym.scancode;
                            for (int i = 0; i < NUM_KEY_FUNCTIONS; i++)
                            {
                                if ( i == activeButton - 1 ) continue;
                                else if ( keyScanCode[i] == catchKey.key.keysym.scancode )
                                {
                                    keyScanCode[i] = SDL_SCANCODE_UNKNOWN;
                                }
                            }
                            activeButton = 0;
                            keybindScreen = false;
                            break;
                        }
                        else if ( catchKey.type == SDL_MOUSEBUTTONUP )
                        {
                            keyScanCode[activeButton - 1] = SDL_SCANCODE_UNKNOWN;
                            activeButton = 0;
                            keybindScreen = false;
                            break;
                        }
                    }
                }
                // renderText( to_string( nextBoxes ), LENGTH_UNIT * 42, LENGTH_UNIT * 29, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
                break;
            }
        }
    }
    else if ( scene == INGAME_SETTINGS )
    {
        renderText( "SETTINGS" , WINDOW_WIDTH / 2, LENGTH_UNIT * 8, false, CENTER, MIDDLE, 2, SDL_Color {255, 255, 255} );
        int tmpSlider = -1;
        renderText( "BACKGROUND MUSIC" , LENGTH_UNIT * 16, LENGTH_UNIT * 12, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
        renderText( to_string( bgmVolume ), LENGTH_UNIT * 48, LENGTH_UNIT * 12, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
        tmpSlider = adjustmentSlider( bgmVolume, LENGTH_UNIT * 40, LENGTH_UNIT * 12, activeSlider == BGM_VOLUME );
        if ( adjusted != PRESSED ) activeSlider = -1;
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
            anyActive = true;
        }
        
        renderText( "SFX" , LENGTH_UNIT * 16, LENGTH_UNIT * 15, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
        renderText( to_string( sfxVolume ), LENGTH_UNIT * 48, LENGTH_UNIT * 15, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
        tmpSlider = adjustmentSlider( sfxVolume, LENGTH_UNIT * 40, LENGTH_UNIT * 15, activeSlider == SFX_VOLUME );
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
            anyActive = true;
        }

        renderText( "SCALE PLAYFIELD ELEMENTS" , LENGTH_UNIT * 16, LENGTH_UNIT * 18, false, LEFT, MIDDLE, 1, SDL_Color {255, 255, 255} );
        renderText( to_string( (int)(playfieldScale * 100) ) + "%", LENGTH_UNIT * 48, LENGTH_UNIT * 18, false, CENTER, MIDDLE, 1, SDL_Color {255, 255, 255} );
        tmpSlider = adjustmentSlider( (playfieldScale - 0.5) / 0.0084, LENGTH_UNIT * 40, LENGTH_UNIT * 18, activeSlider == PLAYFIELD_SCALE );
        if ( tmpSlider != -1 && (activeSlider == -1 || activeSlider == PLAYFIELD_SCALE) )
        {
            activeButton = PLAYFIELD_SCALE + 1;
            if ( adjusted == PRESSED )
            {
                playfieldScale = 0.5 + 0.0084 * tmpSlider;
                activeSlider = PLAYFIELD_SCALE;
                applySettings(PLAYFIELD_SCALE);
            }
            tmpSlider = -1;
            anyActive = true;
        }
    }
    if (!anyActive) activeButton = 0;
}

void taskManager()
{
    //Single player mod
    int mod[] = { 1, 40, 2, false };

    //Number of players & current game mode
    int players, gameMode;

    //Transition status
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
            case MAIN_MENU_SETTINGS:
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

bool handleKeybindButton( string content, int mouse_x, int mouse_y, int x, int y, int w, int h, SDL_Color bg, SDL_Color txtColor)
{
    renderKeybindButton( content, x, y, w, h, bg, txtColor );
    return (mouse_x >= x - w / 2 && mouse_x <= x + w / 2 && mouse_y >= y - h / 2 && mouse_y <= y + h / 2);
}

void updateHighScore( int mode, int score, int line, int time )
{
    enum result { SCORE, LINE, TIME };
    int place = 5;
    switch( mode )
    {
        //Ranks classic/master/mystery results by score
        case CLASSIC:
        case MASTER:
        case MYSTERY:
            for ( int i = 4; i > -1; i-- )
            {
                if ( hiscore[mode][i][SCORE] < score ) place --;
                else if ( hiscore[mode][i][SCORE] > score  || hiscore[mode][i][LINE] < line ) break;
                else if ( hiscore[mode][i][LINE] > line || hiscore[mode][i][TIME] > time) place --;
                else break;
            }
            break;
        //Ranks sprint results by time/line
        case SPRINT:
            for ( int i = 4; i > -1; i-- )
            {
                if (    (hiscore[mode][i][LINE] == 0 && line > 0 || (line == 0 && hiscore[mode][i][SCORE] < score)) ||
                        line != 0 &&   (hiscore[mode][i][TIME] / hiscore[mode][i][LINE] > time/line ||
                                        (hiscore[mode][i][TIME] / hiscore[mode][i][LINE] == time/line && (  hiscore[mode][i][LINE] < line ||
                                                                                                            (hiscore[mode][i][LINE] == line && hiscore[mode][i][score] < score)
                                                                                                         )
                                        )
                                       )
                    ) place --;
                else break;
            }
            break;
        //Ranks blitz results by score/time
        case BLITZ:
            for ( int i = 4; i > -1; i-- )
            {
                if (    hiscore[mode][i][TIME] == 0 ||
                        hiscore[mode][i][SCORE] / hiscore[mode][i][TIME] < score/time ||
                        hiscore[mode][i][SCORE] / hiscore[mode][i][TIME] == score/time && ( hiscore[mode][i][SCORE] > score ||
                                                                                            (hiscore[mode][i][SCORE] == score && hiscore[mode][i][LINE] > line)
                                                                                          )
                   ) place --;
                else break;
            }
            break;
    }
    if ( place < 5 )
    {
        vector<int> newscore = {score, line, time};
        hiscore[mode].pop_back();
        hiscore[mode].insert( hiscore[mode].begin() + place, newscore );
    }
}

void loadHighScore()
{
    ifstream hifile(HIGHSCORE_DIR);
    if ( hifile.is_open() )
    {
        for ( int i = 0; i < 5; i++ )
            for ( int j = 0; j < 5; j++ )
            {
                string data;
                getline(hifile, data);
                istringstream ss(data);
                for ( int k = 0; k < 3; k++ ) ss >> hiscore[i][j][k];
            }
        hifile.close();
    }
    else saveHighScore();
}

void saveHighScore()
{
    ofstream hifile(HIGHSCORE_DIR);
    for ( int i = 0; i < 5; i++ )
        for ( int j = 0; j < 5; j++ )
        {
            for ( int k = 0; k < 3; k++ ) hifile << hiscore[i][j][k] << " ";
            hifile << endl;
        }
    hifile.close();
}

void multiplayerManager( int scene, int &changeScene, int mouse_x, int mouse_y, int &activeButton, bool isClicked, SDL_Keycode key, string text )
{
    //Gets player name for displaying with other players
    if ( playerName == "" )
    {   
        static bool needEdit = false;
        if ( !needEdit )
        {
            needEdit = true;
            SDL_StartTextInput();
        }
        bool done = renderTextInputBox( "ENTER YOUR NAME", "SET", tmpName, mouse_x, mouse_y, isClicked, key, text );
        if ( done ) {playerName = tmpName; tmpName = ""; SDL_StopTextInput(); needEdit = false;}
    }
    //Other events if player already has a name
    else if (changeScene == scene ) switch( scene )
    {
        case MULTI_MENU:
            renderMultiMenu( mouse_x, mouse_y, activeButton, isClicked );
            if ( isClicked )
            {
                switch( activeButton )
                {
                    case 0:
                        changeScene = CREATE_SERVER ;
                        mInfo.serverName = playerName + "'s server";
                        mInfo.maxPlayers = 4;
                        mInfo.gameMode = SCORE;
                        mInfo.lvlSpd = 1;
                        mInfo.winCount = 1;
                        break;
                    case 1:
                        changeScene = JOIN_SERVER ;
                        break;
                    case 2:
                        activeButton = -1;
                        tmpName = playerName;
                        playerName = "";
                        break;
                }
            } 
            break;
        case CREATE_SERVER:
        {
            //Flag to enable/disable text input capture of SDL event
            static bool needEdit = false;
            if ( !needEdit )
            {
                needEdit = true;
                SDL_StartTextInput();
            }

            //Setting rules
            bool start = renderMatchSettings( mouse_x, mouse_y, isClicked, key, text  );
            int status;
            status = adjustmentButton( LENGTH_UNIT * 40, LENGTH_UNIT * 17, mInfo.maxPlayers == 2, mInfo.maxPlayers == 4 );
            if ( isClicked ) mInfo.maxPlayers += status;

            status = adjustmentButton( LENGTH_UNIT * 40, LENGTH_UNIT * 20, mInfo.gameMode == SCORE, mInfo.gameMode == ATTACK );
            if ( isClicked ) mInfo.gameMode += status;
            
            int limitSpd = (mInfo.gameMode == SCORE) ? 19 : (mInfo.gameMode == ATTACK ? 10 : 15);
            if ( mInfo.lvlSpd > limitSpd ) mInfo.lvlSpd = limitSpd;
            status = adjustmentButton( LENGTH_UNIT * 40, LENGTH_UNIT * 23, mInfo.lvlSpd == 1, mInfo.lvlSpd == limitSpd );
            if ( isClicked ) mInfo.lvlSpd += status;
            
            status = adjustmentButton( LENGTH_UNIT * 40, LENGTH_UNIT * 26, mInfo.winCount == 1, mInfo.winCount == 10 );
            if ( isClicked ) mInfo.winCount += status;

            //Handle create event
            if ( isClicked && start )
            {
                changeScene = MULTI_LOBBY;
                server.createServer();
                isHost = true;
                playerList.push_back( playerInfo {playerName, server.getIPAddressString(), true} );
                SDL_StopTextInput(); needEdit = false;
            }
        }
            break;
        case JOIN_SERVER:
        {
            static bool findServer = false;
            if (!findServer)
            {
                client.searchServer();
                findServer = true;
            }
            static int currPage = 0;
            static int selected = -1;
            
            //Handles displaying & browsing server list
            renderJoinServer( mouse_x, mouse_y, activeButton, selected, currPage, isClicked, client.address, client.serverName );
            int totalPages = client.address.size() / 6 + ( client.address.size() % 6 == 0 ? 0 : 1);
            if ( totalPages == 0 ) totalPages = 1;
            renderText( "Page " + to_string( currPage + 1 ) + "/" + to_string( totalPages ), WINDOW_WIDTH / 2, LENGTH_UNIT * 27, false, CENTER, MIDDLE, 1 );
            int changePage = adjustmentButton( WINDOW_WIDTH / 2, LENGTH_UNIT * 27, currPage == 0, currPage == totalPages - 1 );
            if ( isClicked && changePage != 0 ) {currPage += changePage; selected = -1;}
            
            //Handles actions: Refresh the list or Join a server
            enum btnFn { REFRESH = 100, JOIN };
            if ( isClicked )
            {
                if ( activeButton == REFRESH ) {findServer = false; currPage = 0; selected = -1;}
                else if ( activeButton == JOIN )
                {
                    findServer = false;
                    changeScene = MULTI_LOBBY;
                    isHost = false;
                    client.connectToServer(selected);
                    selected = -1;
                    activeButton = -1;
                    currPage = 0;
                }
            }
        }
            break;
        case MULTI_LOBBY:
        {
            //Count the loop cycle for some tasks need repeating after some intervals
            static int count = 0;
            renderLobby( mouse_x, mouse_y, activeButton );
            if ( changeScene == scene )
            {
                
                if ( isHost )
                {
                    //Pings the clients to see if they still hold the connection
                    if ( count == 1000 )
                    {
                        server.pingClient();
                        count = 0;
                    }
                    if ( isClicked && activeButton > 0)
                    {
                        // Start Button = 5
                        if ( activeButton == 5 )
                        {
                            for ( int i = 1; i < playerList.size(); i++ ) server.makeMsg( "start", i - 1 );
                            changeScene = INGAME;
                        }
                        else
                        {
                            //Kick player i button = number i (in list: 1, 2, 3)
                            server.makeMsg( "kick", activeButton - 1 );
                        }
                        server.sendToClient();
                    }
                    //Broadcast server's info & accept join requests every interval
                    if (playerList.size() < mInfo.maxPlayers && count % 25 == 0) 
                    {
                        server.broadcastInvitation();
                        server.acceptConnection();
                    }

                    //Listens to clients actions: Ready/Cancel ready or quit.
                    server.receive();
                    Sleep(1);

                    //Processes the actions & Echoes the players' msg to other players.
                    for ( int i = playerList.size() - 1; i > 0; i-- )
                    {
                        string msg = server.getMsg( i - 1 );
                        if ( msg == "quit" )
                        {
                            server.closeClientSocket(i - 1);
                        }
                        else if (msg == "ready")
                        {
                            playerList[i].ready = true;
                            for ( int j = 0; j < server.getClientNum(); j++ )
                            {
                                if ( j + 1 == i ) continue;
                                else server.makeMsg( to_string(i) + "ready", j );
                            }
                        }
                        else if (msg == "nready")
                        {
                            playerList[i].ready = false;
                            for ( int j = 0; j < server.getClientNum(); j++ )
                            {
                                if ( j + 1 == i ) continue;
                                else server.makeMsg( to_string(i) + "nready", j );
                            }
                        }
                        server.sendToClient();
                    }
                }
                else //If this is a client
                {
                    //Handles ready actions & sends the command to the server
                    if ( isClicked && activeButton != 0 )
                    {
                        if ( !playerList[client.getPosition()].ready )
                        {
                            client.sendToServer( "ready" );
                            playerList[client.getPosition()].ready = true;
                        }
                        else
                        {
                            client.sendToServer( "nready" );
                            playerList[client.getPosition()].ready = false;
                        }
                    }

                    //Handles messages from the server
                    if ( client.isConnected() )
                    {
                        if (count == 1000)
                        {
                            client.pingServer();
                            count = 0;
                        }
                        Sleep(1);
                        client.receive();
                        string msg = client.getMsg();
                        if ( msg == "start" ) changeScene = INGAME;
                        else if ( msg == "kick" ) { client.closeSocket(); changeScene = MULTI_MENU; }
                        else if ( msg.length() > 0 )
                        {
                            if ( msg[0] == 'n' )
                            {
                                msg = msg.substr( 2 );
                                playerInfo tmpPl = { "", "", false };
                                for ( int i = 0; i < msg.length(); i++ )
                                {
                                    if ( msg[i] == delimiter )
                                    {
                                        tmpPl.address = msg.substr( i + 1 );
                                        playerList.push_back( tmpPl );
                                        break;
                                    }
                                    else tmpPl.name += msg[i];
                                }
                            }
                            else
                            {
                                int pl = msg[0] - '0';
                                if ( pl == 0 ) { client.closeSocket(); changeScene = MULTI_MENU; }
                                else
                                {
                                    string cmd = msg.substr(1);
                                    if ( cmd == "quit" ) {playerList.erase( playerList.begin() + pl ); client.changePosition(); }
                                    else if ( cmd == "ready" ) playerList[pl].ready = true;
                                    else if ( cmd == "nready" ) playerList[pl].ready = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        client.closeSocket();
                        changeScene = MULTI_MENU;
                    }
                }
                //Done loop cycle.
                count++;
            }
        }
    }
}