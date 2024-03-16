#include "rendering.hpp"
#include <SDL_image.h>
#include <iostream>
using namespace std;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

SDL_Window *game_window = NULL;

void renderBoard( PlayBoard pb )
{
    //Store the max and min of x and y of a pixel in the playfield
    const int LEFT_X = ( WINDOW_WIDTH - pb.getWidth() ) / 2;
    const int RIGHT_X = LEFT_X + pb.getWidth();
    const int TOP_Y = ( WINDOW_HEIGHT - pb.getHeight() ) / 2;
    const int BOTTOM_Y = TOP_Y + pb.getHeight();

    //Draw board background color
    // SDL_SetRenderDrawColor( renderer, 0x00, 0xFF, 0xFF, 0xFF );
    SDL_Rect board { LEFT_X, TOP_Y , pb.getWidth(), pb.getHeight() };
    // SDL_RenderDrawRect( renderer, &board );

    //Draw board gridlines
    SDL_SetRenderDrawColor( renderer, 0x22, 0x22, 0x22, 0xFF );
    for( int i = 1; i < WIDTH_BY_TILE; i++ )
    {
        SDL_RenderDrawLine( renderer, LEFT_X + TILE_WIDTH * i, TOP_Y, LEFT_X + TILE_WIDTH * i, TOP_Y + pb.getHeight() );
    }
    for( int i = 1; i < HEIGHT_BY_TILE - HIDDEN_ROW; i++ )
    {
        SDL_RenderDrawLine( renderer, LEFT_X, TOP_Y + TILE_WIDTH * i, LEFT_X + pb.getWidth(), TOP_Y  + TILE_WIDTH * i );
    }

    //Draw board borders
    SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    for ( int i = -1; i < 2; i++ )
    {
        //Explain: 
        //Use loop to draw many lines next to each other to create thick border
        //Add abs(i) to round line ends

        //Left border
        SDL_RenderDrawLine( renderer, LEFT_X + i - 2, TOP_Y + abs(i), LEFT_X + i - 2, TOP_Y + pb.getHeight() + i );
        //Right border
        SDL_RenderDrawLine( renderer, LEFT_X + pb.getWidth() + i + 1, TOP_Y + abs(i), LEFT_X + pb.getWidth() + i + 1, TOP_Y + pb.getHeight() - i );
        //Bottom border
        SDL_RenderDrawLine( renderer, LEFT_X + i, TOP_Y + pb.getHeight() + i + 1, LEFT_X + pb.getWidth() - i, TOP_Y + pb.getHeight() + i + 1);
    }

    // Draw pieces on the board;
    for ( int row = 0; row < HEIGHT_BY_TILE; row++ )
    {
        for ( int col = 0; col < WIDTH_BY_TILE; col++ )
        {
            int cellState = pb.getCellState( row, col );
            if ( cellState > 0)
            {
                tileSpriteSheet.render( LEFT_X + TILE_WIDTH * col, BOTTOM_Y - TILE_WIDTH * ( row + 1 ), TILE_WIDTH, TILE_WIDTH, &tileSpriteClips[ cellState ] );
            }
        }
    }
}

void renderCurrentTetrimino( PlayBoard pb, Tetrimino tetr )
{
    const int BOTTOM_LEFT_X = ( WINDOW_WIDTH - pb.getWidth() ) / 2;
    const int BOTTOM_LEFT_Y = ( WINDOW_HEIGHT + pb.getHeight() ) / 2;

    for ( int row = 0; row < tetr.getContainerSize(); row++ )
    {
        for ( int col = 0; col < tetr.getContainerSize(); col++ )
        {
            if ( tetr.getCellState( row, col ) > 0 )
            {
                tileSpriteSheet.render( BOTTOM_LEFT_X + ( tetr.getCol() + col ) * TILE_WIDTH,
                                        BOTTOM_LEFT_Y - ( tetr.getRow() + row + 1) * TILE_WIDTH,
                                        TILE_WIDTH, TILE_WIDTH,
                                        &tileSpriteClips[ tetr.getCellState( row, col ) ]);
            }
        }
    }
}

void renderTetriminoQueue( vector<Tetrimino> Tqueue )
{

}

void renderHeldTetrimino( Tetrimino tetr )
{

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
        //Create game window
        game_window = SDL_CreateWindow( "Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
        if ( game_window == NULL )
        {
            cout << "Failed to create window" << endl;
            success = false;
        }
        else 
        {
            //Create renderer
            renderer = SDL_CreateRenderer( game_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
            if ( renderer == NULL )
            {
                cout << "Failed to create renderer" << endl;
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
            }
        }
    }
    return success;
}


void loadMedia()
{
    const string TILE_SPRITE_SHEET_DIR = "Tile_sheet.png";
    const string AUDIO_DIR = "/Tile_sheets.png";
    if ( !tileSpriteSheet.loadFromFile( TILE_SPRITE_SHEET_DIR ) )
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

}

void close()
{
    tileSpriteSheet.free();
    //Destroy window
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( game_window );
    game_window = NULL;
    renderer = NULL;

    //Quit SDL
    SDL_Quit();
}