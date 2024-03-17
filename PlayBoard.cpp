#include "PlayBoard.hpp"
#include <algorithm>
#include <iostream>
using namespace std;


PlayBoard::PlayBoard()
{
    boardState = vector<vector<int>>( HEIGHT_BY_TILE, vector<int>( WIDTH_BY_TILE, 0 ) );
    w = TILE_WIDTH * WIDTH_BY_TILE;
    h = TILE_WIDTH * (HEIGHT_BY_TILE - HIDDEN_ROW);
}

PlayBoard::~PlayBoard() {}

int PlayBoard::getWidth() const
{
    return w;
}

int PlayBoard::getHeight() const
{
    return h;
}

void PlayBoard::modifyCell( int row, int col, int val )
{
    boardState[row][col] = val;
}

int PlayBoard::clearCompletedRow( int upperRow, int lowerRow )
{
    int rowCleared = 0;

    for ( int i = upperRow; i >= max( lowerRow, 0 ); i-- )
    {
        if ( find( boardState[i].begin(), boardState[i].end(), 0 ) == boardState[i].end() )
        {
            boardState.erase( boardState.begin() + i );
            rowCleared++;
        }
    }
    for ( int i = 0; i < rowCleared; i++ ) boardState.push_back( vector<int>( 10, 0 ) );
    return rowCleared;
}

int PlayBoard::getCellState( int row, int col ) const
{
    if ( row < 0 || row >= HEIGHT_BY_TILE || col < 0 || col >= WIDTH_BY_TILE ) return -1;
    return boardState[row][col];
}

void PlayBoard::updateBoard( int upperRow, int lowerRow )
{
    int rowCleared = clearCompletedRow( upperRow, lowerRow );
    line += rowCleared;
    switch(rowCleared)
    {
        case 1:
            score += 100;
            break;
        case 2:
            score += 300;
            break;
        case 3:
            score += 500;
            break;
        case 4:
            score += 800;
    }
}