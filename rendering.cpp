#include "rendering.hpp"
#include <SDL_image.h>
#include <iostream>
#include <cstdlib>
using namespace std;

SDL_Window *game_window;

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

void renderStatistics( const Player& player, Uint32 startMark, int countDownMark )
{
    //Text box's structure:
    //-----LINE_SPACING-----
    //TITLE (left/right aligned)
    //-----LINE_SPACING-----
    //PRIMARY_TEXT (alignment follows TITLE)
    //-----LINE_SPACING-----

    // TITLE_HEIGHT = TILE_WIDTH;
    // LINE_SPACING = TILE_WIDTH / 2;

    const int SIDE_PADDING = TILE_WIDTH;
    const int BOX_HEIGHT = TILE_WIDTH * 4;
    const double PRIMARY_TEXT_SCALE = 3 / 2.0;
    
    //Relative position of text in textbox
    const int PRIMARY_TEXT_Y = TILE_WIDTH * 2;
    const int TITLE_Y = TILE_WIDTH;

    //Textbox's left/right (depends on its textbox) side
    
    //Used for textbox on the left side of playfield 
    const int LEFT_X =  player.getX() - SIDE_PADDING; 
    //Used for textbox on the left side of playfield 
    const int RIGHT_X =  player.getX() + BOARD_WIDTH + SIDE_PADDING;

    //Bottom of playfield
    const int BOTTOM_Y =  player.getY() + BOARD_HEIGHT;

    //Display line cleared
    renderText( "LINES", LEFT_X, BOTTOM_Y + TITLE_Y - BOX_HEIGHT, false, RIGHT, TOP );
    renderText( to_string( player.getLine() ), LEFT_X, BOTTOM_Y + PRIMARY_TEXT_Y - BOX_HEIGHT, true, RIGHT, TOP, PRIMARY_TEXT_SCALE ); 

    //Display level speed
    renderText( "LV SPEED", LEFT_X, BOTTOM_Y + TITLE_Y - ( BOX_HEIGHT * 2 ), false, RIGHT, TOP );
    renderText( to_string( player.getLevel() ), LEFT_X, BOTTOM_Y + PRIMARY_TEXT_Y - ( BOX_HEIGHT * 2 ), true, RIGHT, TOP, PRIMARY_TEXT_SCALE ); 

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
    if ( SDL_GetTicks() - startMark < 3000)
    {
        renderText( to_string( 3 - (SDL_GetTicks() - startMark) / 1000 ), x + w / 2, y + h / 2, true, CENTER, MIDDLE, 3, SDL_Color { 255, 255, 255 } );
        return false;
    }
    else return true;
}

void clearScreen()
{
    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
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
    const int MIN_STATE = 0;
    const int MAX_STATE = 10;
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
            if ( buttonHighlightState[i] < MAX_STATE ) buttonHighlightState[i] ++;
        }
        else if ( buttonHighlightState[i] > MIN_STATE )
        {
            buttonHighlightState[i] --;
        }
        int alpha_val = 255 * buttonHighlightState[i] / MAX_STATE;
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, alpha_val);
        SDL_RenderFillRect( renderer, &buttonBox[i] );

        SDL_Color color { 255 - alpha_val, 255 - alpha_val, 255 - alpha_val };
        renderText( buttonText[i], buttonBox[i].x + TILE_WIDTH, buttonBox[i].y +  buttonBox[i].h / 2, true, LEFT, MIDDLE, 2, color );
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
int currentFloating = 0;
Uint32 spawnMark, waitUntilNextSpawn;
const int MOVE_DELAY = 1000 / 60;
Uint32 moveMark = SDL_GetTicks();
void renderFloatingTetromino( vector<Tetromino> &floating )
{
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
    title.render( TILE_WIDTH * 4, TILE_WIDTH * 3, TILE_WIDTH * 25, TILE_WIDTH * 14 );
}

bool handleStartButton( int mouse_x, int mouse_y, int center_x, int middle_y )
{
    bool isActive = false;
    int width = 12 * TILE_WIDTH, height = 2 * TILE_WIDTH;
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
    int left_x = 2 * TILE_WIDTH, top_y = WINDOW_HEIGHT - 4 * TILE_WIDTH;
    int width = 4 * TILE_WIDTH, height = 2 * TILE_WIDTH;
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
    renderText( "SELECT GAME MODE", TILE_WIDTH * 4, TILE_WIDTH * 6, true, LEFT, BOTTOM, 3 );
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
            SDL_Rect descPanel { TILE_WIDTH * 37, TILE_WIDTH * 8, TILE_WIDTH * 24, TILE_WIDTH * 22 };
            SDL_RenderFillRect( renderer, &descPanel );
            renderText( "DESCRIPTION", TILE_WIDTH * 40, TILE_WIDTH * 14, true, LEFT, BOTTOM, 3 );
            renderText( soloGameModeName[i] + "Description", TILE_WIDTH * 40, TILE_WIDTH * 15, false, LEFT, TOP );
            renderText( "HIGH SCORE", TILE_WIDTH * 40, TILE_WIDTH * 22, true, LEFT, BOTTOM, 3 );
            
            for ( int j = 1; j <= 5; j++ )
            renderText( to_string(j) + ". 0", TILE_WIDTH * 40, TILE_WIDTH * (22 + j), false, LEFT, TOP );
        }
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderFillRect( renderer, &soloMenuButtonBox[i] );
        renderText( soloGameModeName[i], soloMenuButtonBox[i].x + TILE_WIDTH, soloMenuButtonBox[i].y + soloMenuButtonBox[i].h / 2,
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

// const int LEFT_ADJUSTMENTBUTTON_X =  -TILE_WIDTH * 6;
// const int RIGHT_ADJUSTMENTBUTTON_X = TILE_WIDTH * 5;
// ADJUSTMENT_BUTTON_WIDTH = TILE_WIDTH;
void renderAdjustmentButton( int x, int y, bool disableLeft, bool disableRight )
{
    Texture adjButton;
    adjButton.loadFromFile( "src/media/img/adjustment_button.png" );
    SDL_Rect left {0, 0, 100, 100};
    SDL_Rect right {100, 0, 100, 100};
    if (disableLeft)
    {
        adjButton.setColorMod(50, 50, 50);
        adjButton.render( x + LEFT_ADJUSTMENTBUTTON_X, y, TILE_WIDTH, TILE_WIDTH, &left );
        adjButton.setColorMod(255, 255, 255);
        adjButton.render( x + RIGHT_ADJUSTMENTBUTTON_X, y, TILE_WIDTH, TILE_WIDTH, &right );
    }
    else if (disableRight)
    {
        adjButton.setColorMod(50, 50, 50);
        adjButton.render( x + RIGHT_ADJUSTMENTBUTTON_X, y, TILE_WIDTH, TILE_WIDTH, &right );
        adjButton.setColorMod(255, 255, 255);
        adjButton.render( x + LEFT_ADJUSTMENTBUTTON_X, y, TILE_WIDTH, TILE_WIDTH, &left );
    }
    else
    {
        adjButton.render( x + LEFT_ADJUSTMENTBUTTON_X, y, TILE_WIDTH, TILE_WIDTH, &left );
        adjButton.render( x + RIGHT_ADJUSTMENTBUTTON_X, y, TILE_WIDTH, TILE_WIDTH, &right );
    }
    
}

bool init()
{
    //Initialization status flag
    bool success = true;

    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) < 0 )
    {
        cout << "Failed to initialize SDL" << endl;
        success = false;
    }
    else
    {
        //Set hint
        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

        //Create game window
        game_window = SDL_CreateWindow( "Homemade Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
        if ( game_window == NULL )
        {
            cout << "Failed to create window" << endl;
            success = false;
        }
        else 
        {
            //Create renderer
            renderer = SDL_CreateRenderer( game_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
            if ( renderer == NULL )
            {
                cout << "Failed to create renderer" << endl;
                success = false;
            }
            else
            {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
            }
        }
    }
    return success;
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
}

void loadMedia()
{
    loadMenuElements();
    const string TILE_SPRITE_SHEET_PATH = "src/media/img/Tile_sheet.png";
    const string AUDIO_PATH = "/Tile_sheets.png";

    //Loads sprite sheet
    if ( !tileSpriteSheet.loadFromFile( TILE_SPRITE_SHEET_PATH ) )
    {
        cout << "Failed to load Tile sprite sheet." << endl;
    }
    else
    {
        for ( int i = 0; i < 4; i++ )
        {    
            tileSpriteClips[ i ].x = 300 * i;
		    tileSpriteClips[ i ].y = 0;
		    tileSpriteClips[ i ].w = 300;
		    tileSpriteClips[ i ].h = 300;

            tileSpriteClips[ i + 4 ].x = 300 * i;
		    tileSpriteClips[ i + 4 ].y = 300;
		    tileSpriteClips[ i + 4 ].w = 300;
		    tileSpriteClips[ i + 4 ].h = 300;
        }
    }


    //Load font
    fontBold = TTF_OpenFont("src/media/fonts/gameFontBold.ttf", 30);
    if ( fontBold == NULL )
    {
        cout << "Failed to load font." << endl;
    }
    fontRegular = TTF_OpenFont("src/media/fonts/gameFontRegular.ttf", 30);
    if ( fontRegular == NULL )
    {
        cout << "Failed to load font." << endl;
    }
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

void close()
{
    //Destroy all textures
    tileSpriteSheet.free();
    textTexture.free();
    bgImage.free();
    menuBackground.free();
    //Free font
    TTF_CloseFont( fontBold );
    TTF_CloseFont( fontRegular );
    fontBold = NULL;
    fontRegular = NULL;
    

    //Destroy window
    SDL_DestroyRenderer( renderer );
    renderer = NULL;

    SDL_DestroyWindow( game_window );
    game_window = NULL;

    //Quit SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}