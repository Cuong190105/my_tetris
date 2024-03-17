#include "Tetrimino.hpp"
#include <iostream>
using namespace std;

Tetrimino::Tetrimino()
{
    type = 0;
}

Tetrimino::Tetrimino( int _type, int row, int col )
{
    type = _type;
    currentRotationState = 0;
    currentCol = col;
    //I pieces spawn 1 row lower than usual
    currentRow = row - ( _type == I_PIECE );
    if (_type == I_PIECE || _type == O_PIECE )
    {
        state = vector<vector<int>>( 4, vector<int>(4, 0) );
        containerSize = 4;
    }
    else
    {
        state = vector<vector<int>>( 3, vector<int>( 3, 0 ) );
        containerSize = 3;
    }
    switch ( _type )
    {
        // I piece
        case I_PIECE:
            state[2][0] = _type;
            state[2][1] = _type;
            state[2][2] = _type;
            state[2][3] = _type;
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

int Tetrimino::getType() const
{
    return type;
}

int Tetrimino::getContainerSize() const
{
    return containerSize;
}

int Tetrimino::getCellState( int row, int col ) const
{
    return state[row][col];
}

int Tetrimino::getCol() const
{
    return currentCol;
}

int Tetrimino::getRow() const
{
    return currentRow;
}

void Tetrimino::updateState( int row, int col, int value )
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

bool Tetrimino::checkCollision( const PlayBoard &pb, int rowAdjustment, int colAdjustment ) const {
    for ( int row = 0; row < containerSize; row++ )
    {
        for ( int col = 0; col < containerSize; col++ )
        {
            if ( state[row][col] != 0 && 
                pb.getCellState( currentRow + rowAdjustment + row, currentCol + colAdjustment + col ) != 0)
            {
                return true;
            }
        }
    }
    return false;
}

int Tetrimino::getGhostRow( PlayBoard board ) const
{
    //In some cases, the first (& second) row of tetrimino container is empty.
    //So It's still valid for them to stay out of bound. 
    for ( int i = 0; i <= currentRow + 2; i++ )
    {
        if ( checkCollision( board, - i - 1 ) ) return currentRow - i;
    }
    return -1;
}

void Tetrimino::lockTetrimino( PlayBoard &pb )
{
    type = 0;
    for ( int row = 0; row < containerSize; row++ )
        for ( int col = 0; col < containerSize; col++ )
            if ( state[row][col] != 0 )
            {
                pb.modifyCell( currentRow + row, currentCol + col, state[row][col] );
                state[row][col] = 0;
            }
    pb.updateBoard( currentRow + containerSize - 1, currentRow );
}

void Tetrimino::movePieceHorizontally( PlayBoard board, bool right )
{
    if ( !checkCollision( board, 0, right * 2 - 1 ) )
    {
        currentCol += right * 2 - 1;
    }
}

void Tetrimino::dropPiece( PlayBoard &board, bool isHardDrop, bool &holdable )
{
    bool pieceLocked = false;
    if ( isHardDrop )
    {
        //Hard drop always locks the current piece
        pieceLocked = true;
        currentRow = getGhostRow( board );
    }
    else
    {
        //Soft drop only locks the current piece if it cannot move downwards anymore. 
        if ( !checkCollision( board, -1, 0) ) currentRow -= 1;
        else pieceLocked = true;
    }
    if ( pieceLocked ) 
    {
        lockTetrimino( board );
        holdable = true;
    }

}

void Tetrimino::rotatePiece( PlayBoard board, bool rotateClockwise )
{
    //O piece doesn't need rotating
    if ( type == O_PIECE ) return;

    //tmp is the result of rotating this piece. I piece is declared to spawn 1 row lower,
    //so we add 1 to maintain the current row for the rotation.
    Tetrimino tmp( type, currentRow + ( type == I_PIECE ), currentCol );

    //Rotate state matrix 90deg, direction depending on rotateClockwise flag
    for ( int i = 0; i < containerSize; i++ )
    {
        for ( int j = 0; j < containerSize; j++ )
        {
            if ( rotateClockwise )
            {
                tmp.updateState( containerSize - j - 1, i, state[i][j]); 
            }
            else
            {
                tmp.updateState( j, containerSize - i - 1, state[i][j]);
            }
        }
    }

    /**
     * Stores data using for adjusting the position of the rotated piece
     * if it collides with other pieces or the board. A piece fails to rotate if all 5 adjusted positions
     * are invalid. A dataset contains 5 pairs of integers {colAdjustment, rowAdjustment}.

     * For counterclockwise rotation from state i+1 to i, take the wall kick dataset of
     * rotation from state i to i+1 then multiply all the elements with -1.
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
        int colAdjustments = wallKickAdjustments[i][0] * coeff;
        int rowAdjustments = wallKickAdjustments[i][1] * coeff;
        if ( !tmp.checkCollision( board, rowAdjustments, colAdjustments ) )
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
