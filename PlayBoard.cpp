#include "PlayBoard.hpp"
#include <algorithm>
#include <iostream>
using namespace std;

const int WIDTH_BY_TILE = 10;
const int HEIGHT_BY_TILE = 30;
int TILE_WIDTH = LENGTH_UNIT * playfieldScale;
const int HIDDEN_ROW = HEIGHT_BY_TILE  - 20;
int BOARD_WIDTH = WIDTH_BY_TILE * TILE_WIDTH;
int BOARD_HEIGHT = ( HEIGHT_BY_TILE - HIDDEN_ROW ) * TILE_WIDTH;
PlayBoard::PlayBoard()
{
    boardState = vector<vector<int>>( HEIGHT_BY_TILE, vector<int>( WIDTH_BY_TILE, 0 ) );
}

PlayBoard::~PlayBoard() {}

void PlayBoard::modifyCell( int row, int col, int val )
{
    boardState[row][col] = val;
}

int PlayBoard::getCellState( int row, int col ) const
{
    if ( row < 0 || row >= HEIGHT_BY_TILE || col < 0 || col >= WIDTH_BY_TILE ) return -1;
    return boardState[row][col];
}

int PlayBoard::completedRow( int upperRow, int lowerRow )
{
    int rowCleared = 0;
    for ( int i = upperRow; i >= max( lowerRow, 0 ); i-- )
    {
        if ( find( boardState[i].begin(), boardState[i].end(), 0 ) == boardState[i].end() )
        {
            rowCleared++;
            for ( int j = 0; j < WIDTH_BY_TILE; j++ )
            {
                boardState[i][j] = CLEAR;
            }
        }
    }
    return rowCleared;
}

void PlayBoard::deleteClearedCell()
{
    vector<int> fallRow(WIDTH_BY_TILE, 0);
    for ( int i = 0; i < HEIGHT_BY_TILE; i++ )
    {
        for ( int j = 0; j < WIDTH_BY_TILE; j++ )
        {
            if ( boardState[i][j] == CLEAR) fallRow[j]++;
            else if ( fallRow[j] > 0 ) swap( boardState[i][j], boardState[i-fallRow[j]][j]);
        }
    }
    for ( int i = 0; i < WIDTH_BY_TILE; i++)
    {
        for (int j = 0; j < fallRow[i]; j++)
        {
            boardState[HEIGHT_BY_TILE - 1 - j][i] = 0;
        }
    }
}

void PlayBoard::addRow( int pos )
{
    boardState.insert( boardState.begin() + pos, vector<int>(WIDTH_BY_TILE, 0) );
}

void PlayBoard::removeRow( int pos )
{
    boardState.erase( boardState.begin() + pos );
}