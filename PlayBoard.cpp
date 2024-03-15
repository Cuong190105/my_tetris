#include "PlayBoard.hpp"
#include <algorithm>
using namespace std;


PlayBoard::PlayBoard()
{
    boardState = vector<vector<int>>( HEIGHT_BY_TILE, vector<int>( WIDTH_BY_TILE, 0 ) );
    w = TILE_WIDTH * WIDTH_BY_TILE;
    h = TILE_WIDTH * (HEIGHT_BY_TILE - HIDDEN_ROW);
}

PlayBoard::~PlayBoard() {}

int PlayBoard::getWidth()
{
    return w;
}

int PlayBoard::getHeight()
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

    for ( int i = upperRow; i >= lowerRow; i-- )
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

int PlayBoard::getCellState( int row, int col )
{
    if ( row < 0 || row >= HEIGHT_BY_TILE || col < 0 || col >= WIDTH_BY_TILE ) return -1;
    return boardState[row][col];
}

