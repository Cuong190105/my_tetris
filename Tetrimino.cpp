#include "Tetrimino.hpp"
#include "logic.hpp"

//7 types of tetrimino block
enum Ttype { I_PIECE = 1, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE };

Tetrimino::Tetrimino(int _type)
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

Tetrimino::~Tetrimino() {}

int Tetrimino::getContainerSize()
{
    return containerSize;
}

int Tetrimino::getCellState( int row, int col )
{
    return state[row][col];
}

int Tetrimino::getCol()
{
    return currentCol;
}

int Tetrimino::getRow()
{
    return currentRow;
}

void Tetrimino::updateState( int col, int row, int value )
{
    state[row][col] = value;
}

void Tetrimino::updateCol( int col )
{
    currentCol = col;
}

void Tetrimino::updateRow( int row )
{
    currentRow = row;
}

int Tetrimino::getGhostRow( PlayBoard board )
{
    for ( int i = 0; i < currentRow; i++ )
    {
        bool collided = false;
        for ( int j = 0; j < containerSize && collided == false; j++ )
        {
            for ( int k = 0; k < containerSize; k++)
            {
                if ( state[j][k] != 0 && board.getCellState( i + j, k + currentCol) != 0 )
                {
                    collided = true;
                    break;
                }
            }
        }
        if ( !collided )
        {
            return i;
        }
    }
    return -1;
}

bool Tetrimino::movePieceHorizontally( PlayBoard board, bool right )
{
    if ( !checkCollision( board, *this, 0, right * 2 - 1 ) )
    {
        currentCol += right * 2 - 1;
        return true;
    }
    return false;
}

void Tetrimino::dropPiece( PlayBoard board, bool isHardDrop )
{
    if ( isHardDrop )
    {
        currentRow = getGhostRow( board );
    }
    else
    {
        currentRow -= 1;
    }
}

void Tetrimino::rotatePiece( PlayBoard board, bool rotateClockwise )
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

    /**
     * Stores data using for adjusting the position of the rotated piece
     * if it collides with other pieces or the board. A piece fails to rotate if all 5 adjusted positions
     * are invalid.

     * For counterclockwise rotation from state i+1 to i, take the wall kick dataset of
     * rotation from state i to i+1 then multiply all the coefficients with -1.
     * \return
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
        if ( !checkCollision( board, tmp, rowAdjustments, colAdjustments ) )
        {
            currentRotationState += ( rotateClockwise ? 1 : -1 );
            if ( currentRotationState == 4 ) currentRotationState = 0;
            else if ( currentRotationState == -1 ) currentRotationState = 3;
            for ( int i = 0; i < containerSize; i++ )
            {
                for ( int j = 0; j < containerSize; j++ )
                {
                    state[i][j] = tmp.getCellState( i, j );
                }
            }
            currentRow += rowAdjustments;
            currentCol += colAdjustments;
            return;
        }
    }
}
