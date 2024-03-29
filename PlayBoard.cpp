#include "PlayBoard.hpp"
#include <algorithm>
#include <iostream>
using namespace std;


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

int PlayBoard::countCompletedRow( int upperRow, int lowerRow )
{
    int rowCleared = 0;

    for ( int i = upperRow; i >= max( lowerRow, 0 ); i-- )
    {
        if ( find( boardState[i].begin(), boardState[i].end(), 0 ) == boardState[i].end() )
        {
            rowCleared++;
        }
    }
    return rowCleared;
}

void PlayBoard::updateBoard( int upperRow, int lowerRow )
{
    int row_cleared = 0;
    for ( int i = upperRow; i >= max( lowerRow, 0 ); i-- )
    {
        bool full = true;
        for ( int j = 0; j < WIDTH_BY_TILE; j++ )
        {
            if ( boardState[i][j] <= 0 )
            {
                full = false;
                break;
            }
        }
        if ( full ) {boardState.erase( boardState.begin() + i ); row_cleared++;}
    }
    for ( int i = 0; i < row_cleared; i++ ) boardState.push_back( vector<int>( 10, 0 ) );
}