#include "rendering.hpp"
#include <iostream>
#include <cstdlib>
using namespace std;

void renderText( string text, int x, int y, bool isBold, int Halign, int Valign, double scale, SDL_Color color )
{
    textTexture.loadText( text, ( isBold ? fontBold : fontRegular ), color );
    int top_left_x, top_left_y;
    if ( Halign == LEFT) top_left_x = x;
    else
    {
        int w = textTexture.getWidth() * scale;
        if ( Halign == CENTER )
        {
            top_left_x =  x - (w / 2);
        }
        else if ( Halign == RIGHT )
        {
            top_left_x = x - w;
        }
    }
    if ( Valign == TOP) top_left_y = y;
    else
    {
        int h = textTexture.getHeight() * scale;
        if ( Valign == MIDDLE)
        {
            top_left_y =  y - (h / 2);
        }
        else if ( Valign == BOTTOM )
        {
            top_left_y = y - h;
        }
    }
    textTexture.render( top_left_x, top_left_y, textTexture.getWidth() * scale, textTexture.getHeight() * scale );
}

void renderStatistics( const Player& player, Uint32 startMark, int countDownMark, int lineTarget )
{
    //Text box's structure:
    //-----LINE_SPACING-----
    //TITLE (left/right aligned)
    //-----LINE_SPACING-----
    //PRIMARY_TEXT (alignment follows TITLE)
    //-----LINE_SPACING-----

    // TITLE_HEIGHT = LENGTH_UNIT;
    // LINE_SPACING = LENGTH_UNIT / 2;

    const int SIDE_PADDING = LENGTH_UNIT;
    const int BOX_HEIGHT = LENGTH_UNIT * 3;
    const double PRIMARY_TEXT_SCALE = 3 / 2.0;
    
    //Relative position of text in textbox
    const int PRIMARY_TEXT_Y = LENGTH_UNIT;
    const int TITLE_Y = 0;

    //Textbox's left/right (depends on its textbox) side
    
    //Used for textbox on the left side of playfield 
    const int LEFT_X =  player.getX() - SIDE_PADDING; 
    //Used for textbox on the left side of playfield 
    const int RIGHT_X =  player.getX() + BOARD_WIDTH + SIDE_PADDING;

    //Bottom of playfield
    const int BOTTOM_Y =  player.getY() + BOARD_HEIGHT;

    //Display line cleared
    renderText( "LINES", LEFT_X, BOTTOM_Y + TITLE_Y - BOX_HEIGHT, false, RIGHT, TOP );
    renderText( to_string( player.getLine() ) + (lineTarget != -1 ? "/" + to_string(lineTarget) : ""), LEFT_X, BOTTOM_Y + PRIMARY_TEXT_Y - BOX_HEIGHT, true, RIGHT, TOP, PRIMARY_TEXT_SCALE ); 

    //Display level speed
    renderText( "LV SPEED", LEFT_X, BOTTOM_Y + TITLE_Y - ( BOX_HEIGHT * 2 ), false, RIGHT, TOP );
    renderText( ( player.getMode() == MASTER ? "M" : "") + to_string( player.getLevel() ), LEFT_X, BOTTOM_Y + PRIMARY_TEXT_Y - ( BOX_HEIGHT * 2 ), true, RIGHT, TOP, PRIMARY_TEXT_SCALE ); 

    //Display score
    renderText( "SCORE", RIGHT_X, BOTTOM_Y + TITLE_Y - BOX_HEIGHT, false, LEFT, TOP );
    renderText( to_string( player.getScore() ), RIGHT_X, BOTTOM_Y + PRIMARY_TEXT_Y - BOX_HEIGHT, true, LEFT, TOP, PRIMARY_TEXT_SCALE ); 
    
    // Display time
    string time = "";
    int time_in_seconds = ( SDL_GetTicks() - startMark ) / 1000;
    if ( countDownMark != 0 ) time_in_seconds = countDownMark * 60 - time_in_seconds;
    if ( time_in_seconds / 60 < 10) time += '0';
    time += to_string( time_in_seconds / 60 ) + ":";
    if ( time_in_seconds % 60 < 10) time += '0';
    time += to_string( time_in_seconds % 60 );
    renderText( "TIME", RIGHT_X, BOTTOM_Y + TITLE_Y - ( BOX_HEIGHT * 2 ), false, LEFT, TOP );
    renderText( time, RIGHT_X, BOTTOM_Y + PRIMARY_TEXT_Y - ( BOX_HEIGHT * 2 ), true, LEFT, TOP, PRIMARY_TEXT_SCALE ); 

}

bool displayCountdown( int x, int y, int w, int h, Uint32 startMark)
{
    static bool cdSound = false;
    if ( !cdSound ) { cdSound = true; playSfx(COUNTDOWN); }
    if ( SDL_GetTicks() - startMark < 3000)
    {
        renderText( to_string( 3 - (SDL_GetTicks() - startMark) / 1000 ), x + w / 2, y + h / 2, true, CENTER, MIDDLE, 3, SDL_Color { 255, 255, 255 } );
        return false;
    }
    else { cdSound = false; return true; }
}

void clearScreen()
{
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
    SDL_RenderClear( renderer );
}

//Interval measured in frames
const float HALF_BLINK_DURATION = 5000;
Uint32 blinkMark = SDL_GetTicks();
void renderMenuBackground( bool stop )
{
    static short alpha = 255;
    static short f;
    menuBackground.render();
    SDL_Rect overlay { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, alpha );
    SDL_RenderFillRect( renderer, &overlay );
    if ( SDL_GetTicks() - blinkMark >= HALF_BLINK_DURATION / 255 )
    {
        blinkMark = SDL_GetTicks();
        if ( alpha == 0 ) f = 1;
        else if ( alpha >= 200) f = -1;
        alpha += f;
    }
    if ( stop ) alpha = 255;
}


int buttonHighlightState[MAIN_MENU_BUTTONS];
SDL_Rect buttonBox[MAIN_MENU_BUTTONS];

void renderMainMenuButton( int mouse_x, int mouse_y, int &activeButton )
{
    const string buttonText[] = { "SINGLEPLAYER", "MULTIPLAYER", "SETTINGS", "QUIT" };
    if ( mouse_x >= MAIN_MENU_BUTTON_X && mouse_x <= MAIN_MENU_BUTTON_X + MAIN_MENU_BUTTON_WIDTH )
    {
        for ( int i = 0; i < MAIN_MENU_BUTTONS; i++ )
        {
            if ( mouse_y >= buttonBox[i].y && mouse_y <= buttonBox[i].y + MAIN_MENU_BUTTON_HEIGHT )
            {
                activeButton = i;
                break;
            }
            else activeButton = -1;
        }
    }
    else activeButton = -1;

    for ( int i = 0; i < MAIN_MENU_BUTTONS; i++ )
    {
        if ( i == activeButton )
        {
            if ( buttonHighlightState[i] < 255 )
            {
                buttonHighlightState[i] += 15;
                if ( buttonHighlightState[i] > 255 ) buttonHighlightState[i] = 255;
            }
        }
        else if ( buttonHighlightState[i] > 0 )
        {
            buttonHighlightState[i] -= 15;
            if ( buttonHighlightState[i] < 0 ) buttonHighlightState[i] = 0;
        }
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, buttonHighlightState[i] );
        SDL_RenderFillRect( renderer, &buttonBox[i] );

        SDL_Color color { 255 - buttonHighlightState[i] , 255 - buttonHighlightState[i] , 255 - buttonHighlightState[i]  };
        renderText( buttonText[i], buttonBox[i].x + LENGTH_UNIT, buttonBox[i].y +  buttonBox[i].h / 2, true, LEFT, MIDDLE, 2, color );
    }
}

void renderMenuTetromino( int _x, int _y, const Tetromino &tetr )
{
        //Dimensions of a preview box.
    const int BOX_WIDTH = TILE_WIDTH * 4;
    const int BOX_HEIGHT = TILE_WIDTH * 4;
    //Adjusts Y to center the tetromino in preview box, offsetX is always half the container's dimension. 
    int offsetX, offsetY;

    //Center point of the preview box.
    const int CENTER_X = BOX_WIDTH / 2 + _x;
    const int CENTER_Y = BOX_HEIGHT / 2 + _y;

    if ( tetr.getType() == I_PIECE )    offsetY = TILE_WIDTH * 5 / 2;
    else                                offsetY = TILE_WIDTH * 2;

    offsetX = tetr.getContainerSize() * TILE_WIDTH / 2;

    for ( int row = 0; row < tetr.getContainerSize(); row++ )
        for ( int col = 0; col < tetr.getContainerSize(); col++ )
            if ( tetr.getCellState( row, col ) != 0 )
                tileSpriteSheet.render( CENTER_X - offsetX + col * TILE_WIDTH,
                                        CENTER_Y + offsetY - (row + 1) * TILE_WIDTH,
                                        TILE_WIDTH, TILE_WIDTH,
                                        &tileSpriteClips[ tetr.getCellState( row, col ) ] );
}

const int MAX_FLOAT_PIECE = 5;
int floatSpd[MAX_FLOAT_PIECE], spinAngle[MAX_FLOAT_PIECE];
pair<int, int> position[MAX_FLOAT_PIECE];
bool isVertical[MAX_FLOAT_PIECE];
float pieceScale[MAX_FLOAT_PIECE];
Uint32 spawnMark, waitUntilNextSpawn;
Uint32 moveMark = SDL_GetTicks();
void renderFloatingTetromino( vector<Tetromino> &floating )
{
    const int MOVE_DELAY = 1000 / 60;
    static int currentFloating = 0;
    if ( currentFloating == 0 || ( currentFloating < 5 && SDL_GetTicks() - spawnMark > waitUntilNextSpawn ) )
    {
        floatSpd[currentFloating] = ( rand() % 4 + 2 ) * ( rand() % 2 * 2 - 1 );
        waitUntilNextSpawn = rand() % 2000 + 1000;
        isVertical[currentFloating] = rand() % 2 ;
        if ( isVertical[currentFloating] )
        {
            position[currentFloating] = make_pair(  rand() % ( TILE_WIDTH * 56 ) + TILE_WIDTH * 4,
                                                    floatSpd[currentFloating] > 0 ? TILE_WIDTH * -4 : WINDOW_HEIGHT );
        }
        else
        {
            position[currentFloating] = make_pair(  floatSpd[currentFloating] > 0 ? TILE_WIDTH * -4 : WINDOW_WIDTH,
                                                    rand() % ( TILE_WIDTH * 24 ) + TILE_WIDTH * 4 );
        }
        pieceScale[currentFloating] = (rand() % 11) / 10.f + 1;
        spinAngle[currentFloating] = 0;
        currentFloating++;
        spawnMark = SDL_GetTicks();
    }
    for ( int i = 0; i < currentFloating; i++ )
    {
        if ( ( isVertical[i] && ( position[i].second > WINDOW_HEIGHT || position[i].second < TILE_WIDTH * -4 ) ) ||
            ( !isVertical[i] && (position[i].first > WINDOW_WIDTH || position[i].first < TILE_WIDTH * -4 ) ) )
        {
            for ( int j = i; j < currentFloating - 1; j++ )
            {
                floatSpd[j] = floatSpd[j+1];
                spinAngle[j] = spinAngle[j+1];
                isVertical[j] = isVertical[j+1];
                position[j] = position[j+1];
                pieceScale[j] = pieceScale[j+1];
            }
            floating.erase( floating.begin() + i );
            currentFloating--;
        }
        SDL_Texture *floatPieceHolder = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, TILE_WIDTH * 4, TILE_WIDTH * 4 );
        SDL_SetRenderTarget( renderer, floatPieceHolder );
        SDL_SetTextureBlendMode( floatPieceHolder, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
        SDL_RenderClear( renderer );
        renderMenuTetromino( 0, 0, floating[i] );
        SDL_SetRenderTarget( renderer, NULL );
        SDL_Rect rect { position[i].first, position[i].second, TILE_WIDTH * 4 * pieceScale[i], TILE_WIDTH * 4 * pieceScale[i] };
        SDL_RenderCopyEx( renderer, floatPieceHolder, NULL, &rect, spinAngle[i], NULL, SDL_FLIP_NONE );
        SDL_DestroyTexture( floatPieceHolder );
        floatPieceHolder = NULL;

    }

    if ( SDL_GetTicks() - moveMark >= MOVE_DELAY )
    for ( int i = 0; i < currentFloating; i++ )
    {
        if ( isVertical[i] ) position[i].second += floatSpd[i];
        else position[i].first += floatSpd[i];
        spinAngle[i] ++;
        if ( spinAngle[i] == 360 ) spinAngle[i] = 0;
        moveMark = SDL_GetTicks();
    }
}

void renderGameTitle( Texture& title)
{
    title.render( LENGTH_UNIT * 4, LENGTH_UNIT * 3, LENGTH_UNIT * 25, LENGTH_UNIT * 14 );
}

bool handleStartButton( int mouse_x, int mouse_y, int center_x, int middle_y )
{
    bool isActive = false;
    int width = 12 * LENGTH_UNIT, height = 2 * LENGTH_UNIT;
    short backalpha = 0;
    SDL_Rect hitbox { center_x - width / 2, middle_y - height / 2, width, height };
    if ( mouse_x >= hitbox.x && mouse_x <= hitbox.x + hitbox.w && mouse_y >= hitbox.y && mouse_y <= hitbox.y + hitbox.h )
    {
        backalpha = 255;
        isActive = true;
    }
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, backalpha );
    SDL_RenderFillRect( renderer, &hitbox );
    SDL_Color color { 255-backalpha, 255-backalpha, 255-backalpha };
    renderText( "START", center_x, middle_y, false, CENTER, MIDDLE, 1, color );
    return isActive;
}

bool handleBackButton( int mouse_x, int mouse_y )
{
    bool isActive = false;
    int left_x = 2 * LENGTH_UNIT, top_y = WINDOW_HEIGHT - 4 * LENGTH_UNIT;
    int width = 4 * LENGTH_UNIT, height = 2 * LENGTH_UNIT;
    short backalpha = 0;
    if ( mouse_x >= left_x && mouse_x <= left_x + width && mouse_y >= top_y && mouse_y <= top_y + height )
    {
        backalpha = 255;
        isActive = true;
    }
    SDL_Rect hitbox { left_x, top_y, width, height };
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, backalpha );
    SDL_RenderFillRect( renderer, &hitbox );
    SDL_Color color { 255-backalpha, 255-backalpha, 255-backalpha };
    renderText( "BACK", left_x + width / 2, top_y + height / 2, false, CENTER, MIDDLE, 1, color );
    return isActive;
}


const string soloGameModeName[] = { "CLASSIC", "SPRINT", "BLITZ", "MASTER", "MYSTERY" };
SDL_Rect soloMenuButtonBox[SOLO_MENU_BUTTONS];
void renderSoloMenu( int mouse_x, int mouse_y, int &activeButton )
{
    renderText( "SELECT GAME MODE", LENGTH_UNIT * 4, LENGTH_UNIT * 6, true, LEFT, BOTTOM, 3 );
    if ( mouse_x >= SOLO_MENU_BUTTON_X && mouse_x <= SOLO_MENU_BUTTON_X + SOLO_MENU_BUTTON_WIDTH )
    {
        for ( int i = 0; i < SOLO_MENU_BUTTONS; i++ )
        {
            if ( mouse_y >= soloMenuButtonBox[i].y && mouse_y <= soloMenuButtonBox[i].y + SOLO_MENU_BUTTON_HEIGHT + SOLO_BUTTON_PADDING )
            {
                activeButton = i;
                break;
            }
        }
    } else activeButton = -1;
    for ( int i = 0; i < SOLO_MENU_BUTTONS; i++ )
    {

        if ( i == activeButton )
        {
            const int BORDER_SIZE = SOLO_BUTTON_PADDING;
            SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
            SDL_Rect border = soloMenuButtonBox[i];
            border.x -= BORDER_SIZE;
            border.y -= BORDER_SIZE;
            border.w += BORDER_SIZE * 2;
            border.h += BORDER_SIZE * 2;
            SDL_RenderFillRect( renderer, &border );

            SDL_SetRenderDrawColor( renderer, 0, 0, 0, 200 );
            SDL_Rect descPanel { LENGTH_UNIT * 37, LENGTH_UNIT * 8, LENGTH_UNIT * 24, LENGTH_UNIT * 22 };
            SDL_RenderFillRect( renderer, &descPanel );
            renderText( "DESCRIPTION", LENGTH_UNIT * 40, LENGTH_UNIT * 14, true, LEFT, BOTTOM, 3 );
            renderText( soloGameModeName[i] + "Description", LENGTH_UNIT * 40, LENGTH_UNIT * 15, false, LEFT, TOP );
            renderText( "HIGH SCORE", LENGTH_UNIT * 40, LENGTH_UNIT * 22, true, LEFT, BOTTOM, 3 );
            
            for ( int j = 1; j <= 5; j++ )
            renderText( to_string(j) + ". 0", LENGTH_UNIT * 40, LENGTH_UNIT * (22 + j), false, LEFT, TOP );
        }
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderFillRect( renderer, &soloMenuButtonBox[i] );
        renderText( soloGameModeName[i], soloMenuButtonBox[i].x + LENGTH_UNIT, soloMenuButtonBox[i].y + soloMenuButtonBox[i].h / 2,
                    false, LEFT, MIDDLE, 2, SDL_Color{255,255,255} );
        if ( activeButton == i )    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 100 );
        else                        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 100 );
        SDL_RenderFillRect( renderer, &soloMenuButtonBox[i] );
    }
}

Uint32 transitionMark = SDL_GetTicks();
void renderTransition( bool &transIn )
{
    const int TRANSITION_DURATION = 1000;
    static int transitionAlpha = 255;
    SDL_Rect overlay { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, transitionAlpha );
    SDL_RenderFillRect( renderer, &overlay );
    if ( SDL_GetTicks() - transitionMark >= TRANSITION_DURATION * 5 / 255 )
    {
        transitionMark = SDL_GetTicks();
        if ( transIn && transitionAlpha != 0 )
        {
            transitionAlpha -= 5;
        }
        else if ( !transIn && transitionAlpha != 255)
        {
            transitionAlpha += 5;
        }
        else 
        {
            transIn = !transIn;
        }
    }
}

// const int LEFT_ADJUSTMENTBUTTON_X =  -LENGTH_UNIT * 6;
// const int RIGHT_ADJUSTMENTBUTTON_X = LENGTH_UNIT * 5;
// ADJUSTMENT_BUTTON_WIDTH = LENGTH_UNIT;
void renderAdjustmentButton( int x, int y, bool disableLeft, bool disableRight )
{
    Texture adjButton;
    adjButton.loadFromFile( "src/media/img/adjustment_button.png" );
    SDL_Rect left {0, 0, 100, 100};
    SDL_Rect right {100, 0, 100, 100};
    if (disableLeft)
    {
        adjButton.setColorMod(50, 50, 50);
        adjButton.render( x + LEFT_ADJUSTMENTBUTTON_X, y, LENGTH_UNIT, LENGTH_UNIT, &left );
        adjButton.setColorMod(255, 255, 255);
        adjButton.render( x + RIGHT_ADJUSTMENTBUTTON_X, y, LENGTH_UNIT, LENGTH_UNIT, &right );
    }
    else if (disableRight)
    {
        adjButton.setColorMod(50, 50, 50);
        adjButton.render( x + RIGHT_ADJUSTMENTBUTTON_X, y, LENGTH_UNIT, LENGTH_UNIT, &right );
        adjButton.setColorMod(255, 255, 255);
        adjButton.render( x + LEFT_ADJUSTMENTBUTTON_X, y, LENGTH_UNIT, LENGTH_UNIT, &left );
    }
    else
    {
        adjButton.render( x + LEFT_ADJUSTMENTBUTTON_X, y, LENGTH_UNIT, LENGTH_UNIT, &left );
        adjButton.render( x + RIGHT_ADJUSTMENTBUTTON_X, y, LENGTH_UNIT, LENGTH_UNIT, &right );
    }
    
}

void loadMenuElements()
{
    //Loads main menu background
    const string MENU_BACKGROUND_PATH = "src/media/img/menu_bg.png";
    if ( !menuBackground.loadFromFile( MENU_BACKGROUND_PATH ) )
    {
        cout << "Failed to load menu background" << endl;
    }

    //Loads button hit boxes & initialize highlighting states
    for ( int i = 0; i < MAIN_MENU_BUTTONS; i++ )
    {
        buttonBox[i].x = MAIN_MENU_BUTTON_X;
        buttonBox[i].y = MAIN_MENU_FIRST_BUTTON_Y + ( MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_PADDING ) * i;
        buttonBox[i].w = MAIN_MENU_BUTTON_WIDTH;
        buttonBox[i].h = MAIN_MENU_BUTTON_HEIGHT;
        buttonHighlightState[i] = 0;
    }
    for ( int i = 0; i < SOLO_MENU_BUTTONS; i++ )
    {
        soloMenuButtonBox[i].x = SOLO_MENU_BUTTON_X;
        soloMenuButtonBox[i].y = SOLO_MENU_FIRST_BUTTON_Y + (SOLO_MENU_BUTTON_HEIGHT + SOLO_BUTTON_PADDING) * i;
        soloMenuButtonBox[i].w = SOLO_MENU_BUTTON_WIDTH;
        soloMenuButtonBox[i].h = SOLO_MENU_BUTTON_HEIGHT;
    }
    playBackgroundMusic( MENU );
}

void loadRandomBackground()
{
    int select = rand() % 10 + 1;
    string bgFile = "src/media/img/bg" + to_string(select) + ".png";
    if ( !bgImage.loadFromFile( bgFile ) )
    {
        cout << "Failed to load background." << endl;
    }
}

void renderResultScreen( const Player &player, Uint32 startMark, string time )
{
    const float DURATION = 750;
    renderText("RESULT", player.getX() + BOARD_WIDTH / 2, player.getY() + LENGTH_UNIT * 4, false, CENTER, MIDDLE, 2);
    if (SDL_GetTicks() - startMark > DURATION)
    {
        renderText("Line", player.getX() + LENGTH_UNIT, player.getY() + LENGTH_UNIT * 8, false, LEFT, MIDDLE);
        renderText(to_string(player.getLine()), player.getX() + BOARD_WIDTH - LENGTH_UNIT, player.getY() + LENGTH_UNIT * 8, false, RIGHT, MIDDLE);
    }
    if (SDL_GetTicks() - startMark > DURATION * 2)
    {
        renderText("Speed LV", player.getX() + LENGTH_UNIT, player.getY() + LENGTH_UNIT * 10, false, LEFT, MIDDLE);
        renderText(to_string(player.getLevel()), player.getX() + BOARD_WIDTH - LENGTH_UNIT, player.getY() + LENGTH_UNIT * 10, false, RIGHT, MIDDLE);
    }
    if (SDL_GetTicks() - startMark > DURATION * 3)
    {
        renderText("Time", player.getX() + LENGTH_UNIT, player.getY() + LENGTH_UNIT * 12, false, LEFT, MIDDLE);
        renderText(time, player.getX() + BOARD_WIDTH - LENGTH_UNIT, player.getY() + LENGTH_UNIT * 12, false, RIGHT, MIDDLE);
    }
    if (SDL_GetTicks() - startMark > DURATION * 4)
    {
        renderText("Score", player.getX() + LENGTH_UNIT, player.getY() + LENGTH_UNIT * 14, false, LEFT, MIDDLE);
        renderText(to_string(player.getScore()), player.getX() + BOARD_WIDTH - LENGTH_UNIT, player.getY() + LENGTH_UNIT * 14, false, RIGHT, MIDDLE);
    }
}

int renderRetryScreen( bool &retryLoop, int &scene )
{
    enum RetryMenuButton { RETRY, QUIT };
    SDL_Rect buttons[2];
    string content[] = { "RETRY", "BACK TO MENU" };
    buttons[RETRY] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 16, LENGTH_UNIT * 8, LENGTH_UNIT * 2};
    buttons[QUIT] = SDL_Rect { LENGTH_UNIT * 28, LENGTH_UNIT * 19, LENGTH_UNIT * 8, LENGTH_UNIT * 2 };
    int mouse_x, mouse_y;
    int activeButton = -1;
    SDL_GetMouseState( &mouse_x, &mouse_y );
    if ( mouse_x >= LENGTH_UNIT * 28 && mouse_x <= LENGTH_UNIT * 36)
    {
        if ( mouse_y <= LENGTH_UNIT * 18 && mouse_y >= LENGTH_UNIT * 16) activeButton = RETRY;
        else if ( mouse_y >= LENGTH_UNIT * 19 && mouse_y <= LENGTH_UNIT * 21) activeButton = QUIT;
    }
    for (int i = 0; i < 2; i++)
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

void Player::displayBonus()
{
    const int DURATION = 2000;
    const int FADE = 500;
    Uint32 time = SDL_GetTicks();
    if ( combo > 0 )
    {
        // textTexture.setAlphaMod( time - bonusMark[0] < DURATION - FADE ? 255 : 255 - (time - bonusMark[0] - DURATION + FADE) * 255 / FADE );
        renderText( to_string( combo ) + "x", x - LENGTH_UNIT * 6, y + LENGTH_UNIT * 6, false, RIGHT, BOTTOM, max(2 - (time - bonusMark[0]) / 500.f * 0.5f, 1.5f));
        renderText( "COMBO", x - LENGTH_UNIT, y + LENGTH_UNIT * 6, false, RIGHT, BOTTOM, 1 );
    }
    if ( bonus & T_SPIN )
    {
        // textTexture.setAlphaMod( time - bonusMark[1] < 1500 ? 255 : 255 - (time - bonusMark[1] - DURATION + FADE) * 255 / FADE );
        renderText( "T-SPIN", x - LENGTH_UNIT, y + LENGTH_UNIT * 9, false, RIGHT, BOTTOM, 2, {245, 66, 239} );
        if ( bonus & MINI ) renderText( "Mini", x - LENGTH_UNIT * 8, y + LENGTH_UNIT * 9, false, RIGHT, BOTTOM, 2, {245, 66, 239} );
        if ( time - bonusMark[1] > DURATION ) { bonus -= T_SPIN; if ( bonus & MINI ) bonus -= MINI; }
    }
    if ( bonus & TETRIS )
    {
        // textTexture.setAlphaMod( time - bonusMark[1] < 1500 ? 255 : 255 - (time - bonusMark[1] - DURATION + FADE) * 255 / FADE );
        renderText( "TETRIS", x - LENGTH_UNIT, y + LENGTH_UNIT * 9, false, RIGHT, BOTTOM, 2, {66, 218, 245} );
        if ( time - bonusMark[1] > DURATION ) bonus -= TETRIS;
    }
    if ( (bonus & B2B == B2B) && b2b > 0 )
    {
        // textTexture.setAlphaMod( time - bonusMark[1] < 1500 ? 255 : 255 - (time - bonusMark[1] - DURATION + FADE) * 255 / FADE );
        renderText( "BACK-TO-BACK", x - LENGTH_UNIT, y + LENGTH_UNIT * 11, false, RIGHT, BOTTOM, 1 );
        if ( time - bonusMark[1] > DURATION ) bonus -= B2B;
    }
    if ( bonus & ALLCLEAR )
    {
        // textTexture.setAlphaMod( time - bonusMark[2] < 1500 ? 255 : 255 - (time - bonusMark[2] - DURATION + FADE) * 255 / FADE );
        renderText( "ALL CLEAR", x - LENGTH_UNIT, y + LENGTH_UNIT * 13, false, RIGHT, BOTTOM );
        if ( time - bonusMark[2] > DURATION ) bonus -= ALLCLEAR;
    }
    // textTexture.setAlphaMod( 255 );
}