#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>

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

class Tetrimino {
    private:
        //A 4x4 matrix which stores the state of each tile of 
        //the tetrimino (a playing piece that falls from the top of the board).
        vector<vector<int>> state;

        //Store type of tetrimino (I, L, J, etc)
        int Ttype;

        //Store bottom left corner's current position of
        //tetrimino container (the 4x4 matrix) on the board
        int currentCol, currentRow;
    public:
        Tetrimino(int type)
        {
            Ttype = type;
            state = vector<vector<int>>( 4, vector<int>(4, 0) );
            switch ( type )
            {
                // I piece
                case 1:
                    state[1][0] = type;
                    state[1][1] = type;
                    state[1][2] = type;
                    state[1][3] = type;
                    break;

                // J piece
                case 2:
                    state[2][0] = type;
                    state[1][0] = type;
                    state[1][1] = type;
                    state[1][2] = type;
                    break;

                // L piece
                case 3:
                    state[2][2] = type;
                    state[1][0] = type;
                    state[1][1] = type;
                    state[1][2] = type;
                    break;

                // O piece
                case 4:
                    state[2][1] = type;
                    state[2][2] = type;
                    state[1][1] = type;
                    state[1][2] = type;
                    break;

                // S piece
                case 5:
                    state[1][0] = type;
                    state[1][1] = type;
                    state[2][1] = type;
                    state[2][2] = type;
                    break;

                // Z piece
                case 6:
                    state[2][0] = type;
                    state[2][1] = type;
                    state[1][1] = type;
                    state[1][2] = type;
                    break;

                // T piece
                case 7:
                    state[1][0] = type;
                    state[1][1] = type;
                    state[1][2] = type;
                    state[2][1] = type;
                    break;
            }
        }

        ~Tetrimino() {}

        void rotatePiece( bool rotateClockwise )
        {
            if ( Ttype == 1) 
            {

            }
        }
};

Tetrimino generateNewTetrimino()
{

}

class PlayBoard {
    private:
        const int WIDTH_BY_TILE = 10;
        const int HEIGHT_BY_TILE = 23;

        //3 rows at the top of the board are hidden, at which new tetrimino is spawned.
        const int HIDDEN_ROW = 3;
        
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
            h = TILE_WIDTH * (HEIGHT_BY_TILE - HIDDEN_ROW);
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

            // Draw pieces on the board;
            // for ( int row = 0; row < )
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
        srand(time(NULL));
        //Event handler
        SDL_Event e;

        //Game loop
        while ( !quit )
        {

            //Initialize game & piece bag;
            PlayBoard board;
            queue<Tetrimino> TetriminoQueue;

            //Handle events
            while ( SDL_PollEvent(&e) != 0 ) 
            {
                //Quitting game event
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                // while ( TetriminoQueue.size() < 5 ) {
                //     TetriminoQueue.push( generateNewTetrimino() );
                // }


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