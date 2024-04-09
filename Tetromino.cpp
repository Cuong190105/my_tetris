#include "Tetromino.hpp"
#include <iostream>
using namespace std;

Tetromino::Tetromino()
{
    type = 0;
}

Tetromino::Tetromino( int _type, int row, int col )
{
    type = _type;
    rotationState = 0;
    corrupted = false;
    currentCol = col;
    //I pieces spawn 1 row lower than usual
    currentRow = row - ( _type == I_PIECE );
    if (_type == BOMB_PIECE)state = vector<vector<int>>( 1, vector<int>(1, BOMB_PIECE) );
    else if (_type == I_PIECE || _type == O_PIECE ) state = vector<vector<int>>( 4, vector<int>(4, 0) );
    else state = vector<vector<int>>( 3, vector<int>( 3, 0 ) );
    containerSize = state.size();
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

Tetromino::~Tetromino() {}

void Tetromino::voidPiece()
{
    type = 0;
}

int Tetromino::getType() const
{
    return type;
}

int Tetromino::getContainerSize() const
{
    return containerSize;
}

int Tetromino::getCellState( int row, int col ) const
{
    return state[row][col];
}

int Tetromino::getCol() const
{
    return currentCol;
}

int Tetromino::getRow() const
{
    return currentRow;
}

int Tetromino::getRotationState() const
{
    return rotationState;
}

void Tetromino::modifyCell( int row, int col, int value )
{
    state[row][col] = value;
}

void Tetromino::updateCol( int col )
{
    currentCol = col;
}

void Tetromino::updateRow( int row )
{
    currentRow = row;
}

void Tetromino::updateRotationState( int newState )
{
    if ( newState == 4 ) rotationState = 0;
    else if ( newState == -1 ) rotationState = 3;
    else rotationState = newState;
}

void Tetromino::makeItGiant()
{
    vector<vector<int>> newState ( containerSize * 2, vector<int>(containerSize * 2, 0) );
    for (int i = 0; i < containerSize; i++)
    {
        for (int j = 0; j < containerSize; j++)
        {
            if (state[i][j] > 0)
            {
                newState[2*i][2*j] = state[i][j];
                newState[2*i+1][2*j] = state[i][j];
                newState[2*i][2*j+1] = state[i][j];
                newState[2*i+1][2*j+1] = state[i][j];
            }
        }
    }
    currentCol -= containerSize - 1;
    currentRow -= containerSize;
    containerSize *= 2;
    state = newState;
 }

bool Tetromino::isCorrupted() const { return corrupted; }

void Tetromino::corruptPiece()
{
    corrupted = true;
    int voidCell = rand() % 4 + 1;
    int row = 0, col = 0;
    while (voidCell > 0)
    {
        if (col == containerSize)
        {
            col = 0;
            row++;
        }
        if (row == containerSize) row = 0;
        if (state[row][col] != 0) voidCell--;
        col++;
    }
    state[row][col] = 0;
}