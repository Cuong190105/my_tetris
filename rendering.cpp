#include "rendering.hpp"
#include <SDL_image.h>
#include <iostream>
using namespace std;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

SDL_Window *game_window = NULL;

void renderBoard( const PlayBoard &pb )
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

void renderCurrentTetrimino( const PlayBoard& pb, const Tetrimino& tetr )
{
    //Playfield's bottom left corner's position
    const int BOTTOM_LEFT_X = ( WINDOW_WIDTH - pb.getWidth() ) / 2;
    const int BOTTOM_LEFT_Y = ( WINDOW_HEIGHT + pb.getHeight() ) / 2;

    for ( int row = 0; row < tetr.getContainerSize(); row++ )
    {
        for ( int col = 0; col < tetr.getContainerSize(); col++ )
        {
            if ( tetr.getCellState( row, col ) > 0 )
            {
                int tile_x = BOTTOM_LEFT_X + ( tetr.getCol() + col ) * TILE_WIDTH;
                int tile_y = BOTTOM_LEFT_Y - ( tetr.getRow() + row + 1) * TILE_WIDTH;
                tileSpriteSheet.render( tile_x, tile_y, 
                                        TILE_WIDTH, TILE_WIDTH,
                                        &tileSpriteClips[ tetr.getCellState( row, col ) ]);
                
                //Renders this tile's ghost.
                int ghostOffsetY = ( tetr.getRow() - tetr.getGhostRow( pb ) ) * TILE_WIDTH;
                tileSpriteSheet.render( tile_x, tile_y + ghostOffsetY, 
                                        TILE_WIDTH, TILE_WIDTH, &tileSpriteClips[ 0 ]);
            }
        }
    }
}

void renderPreviewTetrimino( int x, int y, const Tetrimino &tetr )
{
    //Dimensions of a preview box.
    const int BOX_WIDTH = TILE_WIDTH * 5;
    const int BOX_HEIGHT = TILE_WIDTH * 3;

    //Adjusts Y to center the tetrimino in preview box, offsetX is always half the container's dimension. 
    int offsetX, offsetY;

    //Center point of the preview box.
    const int CENTER_X = BOX_WIDTH / 2 + x;
    const int CENTER_Y = BOX_HEIGHT / 2 + y;

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

void renderTetriminoQueue( const PlayBoard &pb, const vector<Tetrimino>& Tqueue )
{
    //Position of the queue container's top left corner
    const int TOP_LEFT_X = ( WINDOW_WIDTH + pb.getWidth() ) / 2;
    const int TOP_LEFT_Y = ( WINDOW_HEIGHT - pb.getHeight() ) / 2;
    
    //Number of preview boxes. A queue can contain up to 5 tetriminos at a time.
    const int BOX_NUMBER = 5;
    const int BOX_HEIGHT = TILE_WIDTH * 3;
    
    for ( int i = 0; i < BOX_NUMBER; i++ )
    {
        renderPreviewTetrimino( TOP_LEFT_X, TOP_LEFT_Y + i * BOX_HEIGHT, Tqueue[i] );
    }
}

void renderHeldTetrimino( const PlayBoard &pb, const Tetrimino &hold )
{
    const int TOP_LEFT_X = ( WINDOW_WIDTH - pb.getWidth() ) / 2 - ( TILE_WIDTH * 5 );
    const int TOP_LEFT_Y = ( WINDOW_HEIGHT - pb.getHeight() ) / 2;
    renderPreviewTetrimino( TOP_LEFT_X, TOP_LEFT_Y, hold );
}

void clearScreen()
{
    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear( renderer );
}

void renderFrame( const PlayBoard &pb, const Tetrimino& tetr, const vector<Tetrimino> &Tqueue )
{
    //Update screen
    renderBoard( pb );
    renderTetriminoQueue ( pb, Tqueue ) ;
    if ( tetr.getType() != 0 ) renderCurrentTetrimino( pb, tetr );
    SDL_RenderPresent( renderer );
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