#include "rendering.hpp"
#include <iostream>
using namespace std;

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
SDL_Renderer *renderer = NULL;
SDL_Window *game_window = NULL;

void renderBoard( PlayBoard pb )
{
    //Draw board background color
    // SDL_SetRenderDrawColor( renderer, 0x00, 0xFF, 0xFF, 0xFF );
    SDL_Rect board { ( WINDOW_WIDTH - pb.getWidth() ) / 2, ( WINDOW_HEIGHT - pb.getHeight() ) / 2, pb.getWidth(), pb.getHeight() };
    // SDL_RenderDrawRect( renderer, &board );

    //Draw board gridlines
    SDL_SetRenderDrawColor( renderer, 0x22, 0x22, 0x22, 0xFF );
    for( int i = 1; i < WIDTH_BY_TILE; i++ )
    {
        SDL_RenderDrawLine( renderer, board.x + TILE_WIDTH * i, board.y, board.x + TILE_WIDTH * i, board.y + pb.getHeight() );
    }
    for( int i = 1; i < HEIGHT_BY_TILE - HIDDEN_ROW; i++ )
    {
        SDL_RenderDrawLine( renderer, board.x, board.y + TILE_WIDTH * i, board.x + pb.getWidth(), board.y  + TILE_WIDTH * i );
    }

    //Draw board borders
    SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    for ( int i = -1; i < 2; i++ )
    {
        //Explain: 
        //Use loop to draw many lines next to each other to create thick border
        //Add abs(i) to round line ends

        //Left border
        SDL_RenderDrawLine( renderer, board.x + i, board.y + abs(i), board.x + i, board.y + pb.getHeight() );
        //Right border
        SDL_RenderDrawLine( renderer, board.x + pb.getWidth() + i, board.y + abs(i), board.x + pb.getWidth() + i, board.y + pb.getHeight() );
        //Bottom border
        SDL_RenderDrawLine( renderer, board.x, board.y + pb.getHeight() + i, board.x + pb.getWidth(), board.y + pb.getHeight()  + i);
    }

    // Draw pieces on the board;
    // for ( int row = 0; row < )
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
            }
        }
    }
    return success;
}


void close()
{
    //Destroy window
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( game_window );
    game_window = NULL;
    renderer = NULL;

    //Quit SDL
    SDL_Quit();
}