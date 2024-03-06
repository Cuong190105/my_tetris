#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>

using namespace std;

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const int TILE_WIDTH = 30;

SDL_Window *game_window = NULL;

SDL_Renderer *renderer = NULL;

class Tile {
    private:
    public:
};

class PlayBoard {
    private:
        const int WIDTH_BY_TILE = 10;
        const int HEIGHT_BY_TILE = 20;
        
        //Width and height of play board when displayed on screen
        int w, h;

        //A matrix to store all tiles placed on the board
        vector<vector<int>> boardStatus;
    public:
        //Play Board Constructor
        PlayBoard()
        {
            boardStatus = vector<vector<int>>( HEIGHT_BY_TILE, vector<int>( WIDTH_BY_TILE, 0 ) );
            w = TILE_WIDTH * WIDTH_BY_TILE;
            h = TILE_WIDTH * HEIGHT_BY_TILE;
        }

        //Destructor
        ~PlayBoard() {}

        void updateBoard()
        {
            
        }

        void renderBoard()
        {
            //Draw board background color
            // SDL_SetRenderDrawColor( renderer, 0x00, 0xFF, 0xFF, 0xFF );
            SDL_Rect board { ( WINDOW_WIDTH - w ) / 2, ( WINDOW_HEIGHT - h ) / 2, w, h };
            // SDL_RenderDrawRect( renderer, &board );

            //Draw board gridlines
            SDL_SetRenderDrawColor( renderer, 0x22, 0x22, 0x22, 0xFF );
            for( int i = 1; i < WIDTH_BY_TILE; i++ )
            {
                SDL_RenderDrawLine( renderer, board.x + TILE_WIDTH * i, board.y, board.x + TILE_WIDTH * i, board.y + h );
            }
            for( int i = 1; i < HEIGHT_BY_TILE; i++ )
            {
                SDL_RenderDrawLine( renderer, board.x, board.y + TILE_WIDTH * i, board.x + w, board.y  + TILE_WIDTH * i );
            }

            //Draw board borders
            SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
            for ( int i = -1; i < 2; i++ )
            {
                //Explain: 
                //Use loop to draw many lines next to each other to create thick border
                //Add abs(i) to round line ends

                //Left border
                SDL_RenderDrawLine( renderer, board.x + i, board.y + abs(i), board.x + i, board.y  + h );
                //Right border
                SDL_RenderDrawLine( renderer, board.x + w + i, board.y + abs(i), board.x + w + i, board.y  + h );
                //Bottom border
                SDL_RenderDrawLine( renderer, board.x, board.y + h + i, board.x + w, board.y  + h  + i);
            }
        }
};

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

int main(int argv, char **args) {
    //Initialize SDL & Create window
    if ( !init() ) {
        cout << "Failed to initialize" << endl;
    }
    else
    {
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        while ( !quit )
        {
            PlayBoard board;

            //Handle events
            while ( SDL_PollEvent(&e) != 0 ) 
            {
                //Quitting game event
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }

                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
                SDL_RenderClear( renderer );

                board.renderBoard();
                

                //Update screen
                SDL_RenderPresent( renderer );
            }
        }
    }
    close();
    return 0;
}