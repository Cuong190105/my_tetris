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

enum Ttype { I_PIECE, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE };
class Tile {
    private:
    public:
};

class PlayBoard {
    private:
        const int WIDTH_BY_TILE = 10;
        const int HEIGHT_BY_TILE = 23;

        //3 rows at the top of the board are hidden, at which new tetrimino is spawned.
        const int HIDDEN_ROW = 3;
        
        //Width and height of play board when displayed on screen
        int w, h;

        //A matrix to store all tiles placed on the board
        vector<vector<int>> boardState;
    public:
        //Play Board Constructor
        PlayBoard()
        {
            boardState = vector<vector<int>>( HEIGHT_BY_TILE, vector<int>( WIDTH_BY_TILE, 0 ) );
            w = TILE_WIDTH * WIDTH_BY_TILE;
            h = TILE_WIDTH * (HEIGHT_BY_TILE - HIDDEN_ROW);
        }

        //Destructor
        ~PlayBoard() {}

        void updateBoard()
        {
            
        }

        //Get the state of a cell given its coordinate
        //Return -1 if the corresponding cell is outside the board
        int getCellState( int row, int col )
        {
            if ( row < 0 || row >= HEIGHT_BY_TILE || col < 0 || col >= WIDTH_BY_TILE ) return -1;
            return boardState[row][col];
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
            for( int i = 1; i < HEIGHT_BY_TILE - HIDDEN_ROW; i++ )
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

class Tetrimino {
    private:
        /*
        state:
            A n*n matrix which stores the state of each tile of 
            the tetrimino (a playing piece that falls from the top of the board).
        
        containerSize:
            Width n of the state matrix.

        type:
            Store type of tetrimino (I, L, J, etc).
        
        currentCol, currentRow:
            Store bottom left corner's current position of
            tetrimino container on the board.

        currentRotationState:
            Store current rotation state of tetrimino.
            There are 4 rotation states. Every tetrimino spawns with
            a default state called state 0. State 1, 2, 3 correspond to
            its state when rotated 90, 180 and 270 degrees clockwise from spawn state.
        */
        vector<vector<int>> state;
        int containerSize;
        int type;
        int currentCol, currentRow;
        int currentRotationState;
    public:
        Tetrimino(int _type)
        {
            type = _type;
            currentRotationState = 0;
            if (_type == 0 || _type == 3)
                state = vector<vector<int>>( 4, vector<int>(4, 0) );
            else
                state = vector<vector<int>>( 3, vector<int>( 3, 0 ) );
            switch ( _type )
            {
                // I piece
                case I_PIECE:
                    state[1][0] = _type;
                    state[1][1] = _type;
                    state[1][2] = _type;
                    state[1][3] = _type;
                    break;

                // J piece
                case J_PIECE:
                    state[2][0] = _type;
                    state[1][0] = _type;
                    state[1][1] = _type;
                    state[1][2] = _type;
                    break;

                // L piece
                case L_PIECE:
                    state[2][2] = _type;
                    state[1][0] = _type;
                    state[1][1] = _type;
                    state[1][2] = _type;
                    break;

                // O piece
                case O_PIECE:
                    state[2][1] = _type;
                    state[2][2] = _type;
                    state[1][1] = _type;
                    state[1][2] = _type;
                    break;

                // S piece
                case S_PIECE:
                    state[1][0] = _type;
                    state[1][1] = _type;
                    state[2][1] = _type;
                    state[2][2] = _type;
                    break;

                // Z piece
                case Z_PIECE:
                    state[2][0] = _type;
                    state[2][1] = _type;
                    state[1][1] = _type;
                    state[1][2] = _type;
                    break;

                // T piece
                case T_PIECE:
                    state[1][0] = _type;
                    state[1][1] = _type;
                    state[1][2] = _type;
                    state[2][1] = _type;
                    break;
            }
        }

        ~Tetrimino() {}
        
        vector<vector<int>> getState()
        {
            return state;
        }

        void updateState( int col, int row, int value )
        {
            state[row][col] = value;
        }

        void updateCol( int col )
        {
            currentCol = col;
        }

        void updateRow( int row )
        {
            currentRow = row;
        }

        bool checkCollision( PlayBoard board, int rowAdjustment = 0, int colAdjustment = 0 ) {
            for ( int i = 0; i < containerSize; i++ )
            {
                for ( int j = 0; j < containerSize; j++ )
                {
                    if ( state[i][j] != 0 && board.getCellState( currentRow + rowAdjustment + i, currentCol + colAdjustment + i ) != 0)
                        return true;
                }
            }
            return false;
        }

        void rotatePiece( PlayBoard board, bool rotateClockwise )
        {
            //O piece doesn't need rotating
            if ( type == O_PIECE ) return;

            //tmp is the result of rotating this piece.
            Tetrimino tmp = *this;

            //Rotate state matrix 90deg, direction depending on rotateClockwise flag
            for ( int i = 0; i < containerSize; i++ )
            {
                for ( int j = 0; j < containerSize; j++ )
                {
                    if ( state[i][j] != 0 )
                    {
                        if ( rotateClockwise )
                        {
                            tmp.updateState( containerSize - j, i, state[i][j]); 
                        }
                        else
                        {
                            tmp.updateState( j, containerSize - i, state[i][j]);
                        }
                    }
                }
            }

            /*
            wallKickAdjustments store data using for adjusting the position of the rotated piece
            if it collides with other pieces or the board. A piece fails to rotate if all 5 adjusted positions
            are invalid.

            For counterclockwise rotation from state i+1 to i, take the wall kick dataset of
            rotation from state i to i+1 then multiply all the coefficients with -1.
            */
            vector<vector<int>> wallKickAdjustments;
            const int ADJUSTMENTS_COUNT = 5;

            //coeff = dirCoeff * datasetCoeff
            //dirCoeff = -1 if rotating counterclockwise, else 1
            //datasetCoeff = -1 if the circumstance noted in if-block used for choosing dataset below happens, else 1
            int coeff = rotateClockwise ? 1 : -1;

            if ( type != I_PIECE )
            {
                if ( ( rotateClockwise && currentRotationState < 2 ) || ( !rotateClockwise &&  currentRotationState > 0 && currentRotationState < 3 ) )
                {

                    //This set is for rotation from state 0 to state 1.
                    wallKickAdjustments = vector<vector<int>>
                    {
                        vector<int>{0, 0}, vector<int>{-1, 0},
                        vector<int>{-1, 1}, vector<int>{0, -2},
                        vector<int>{-1, -2}
                    };
                    //For rotation from state 1 to 2, multiply each coefficient with -1.
                    
                    //Magic number 2: 2 rotations where datasetCoeff = -1
                    //1. rotation from state 1 to 2 => currentRotationState = 1, rotateClockwise = 1
                    //2. rotation from state 2 to 1 => currentRotationState = 2, rotateClockwise = 0
                    if ( rotateClockwise + currentRotationState == 2 ) coeff *= -1;

                }
                else
                {
                    //This set is for rotating from state 2 to state 3
                    wallKickAdjustments = vector<vector<int>>
                    {
                        vector<int>{0, 0}, vector<int>{1, 0},
                        vector<int>{1, 1}, vector<int>{0, -2},
                        vector<int>{1, -2}
                    };
                    //For rotation from state 3 to 0, multiply each coefficient with -1.
                    //Some other magic numbers: 0 and 4
                    if ( ( rotateClockwise + currentRotationState ) % 4 == 0 ) coeff *= -1;
                }
            }
            else
            {
                if ( ( rotateClockwise + currentRotationState ) & 1 )
                {
                    //This set is for rotation from state 0 to state 1.
                    //For rotation from state 2 to 3, multiply each coefficient with -1.
                    wallKickAdjustments = vector<vector<int>>
                    {
                        vector<int>{0, 0}, vector<int>{-2, 0},
                        vector<int>{1, 0}, vector<int>{2, -1},
                        vector<int>{1, 2}
                    };
                    if ( rotateClockwise + currentRotationState == 3 ) coeff *= -1;
                }
                else
                {
                    //This set is for rotating from state 1 to state 2.
                    //For rotation from state 3 to 0, multiply each coefficient with -1.
                    wallKickAdjustments = vector<vector<int>>
                    {
                        vector<int>{0, 0}, vector<int>{-1, 0},
                        vector<int>{2, 0}, vector<int>{-1, 2},
                        vector<int>{2, -1}
                    };

                    if ( ( rotateClockwise + currentRotationState ) % 4 == 0 ) coeff *= -1;
                }
            }



            for ( int i = 0; i < ADJUSTMENTS_COUNT; i++ )
            {
                int rowAdjustments = wallKickAdjustments[i][0] * coeff;
                int colAdjustments = wallKickAdjustments[i][1] * coeff;
                if ( !tmp.checkCollision( board, rowAdjustments, colAdjustments ) )
                {
                    currentRotationState += ( rotateClockwise ? 1 : -1 );
                    if ( currentRotationState == 4 ) currentRotationState = 0;
                    else if ( currentRotationState == -1 ) currentRotationState = 3;
                    state = tmp.getState();
                    currentRow += rowAdjustments;
                    currentCol += colAdjustments;
                }
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