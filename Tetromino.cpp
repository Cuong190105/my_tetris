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
    rotationState = newState;
}
