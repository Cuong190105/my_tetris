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

vector<int> PlayBoard::completedRow( int upperRow, int lowerRow )
{
    vector<int> rowCleared;

    for ( int i = upperRow; i >= max( lowerRow, 0 ); i-- )
    {
        if ( find( boardState[i].begin(), boardState[i].end(), 0 ) == boardState[i].end() )
        {
            rowCleared.push_back(i);
        }
    }
    return rowCleared;
}

void PlayBoard::updateBoard( vector<int> rowCleared )
{
    for ( int i = 0; i < rowCleared.size(); i++ )
    {
        boardState.erase( boardState.begin() + rowCleared[i] );
        boardState.push_back( vector<int>(10, 0) );
    }
}