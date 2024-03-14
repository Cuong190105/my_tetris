#include <algorithm>
#include "logic.hpp"
#include "Tetrimino.hpp"
using namespace std;

void updateBoard( PlayBoard pb, Tetrimino tetr )
{
    for ( int i = 0; i < tetr.getContainerSize(); i++ )
    {
        for ( int j = 0; j < tetr.getContainerSize(); j++ )
        {
            if ( tetr.getCellState( i, j ) != 0 )
            {
                pb.modifyCell( i + tetr.getRow(), j + tetr.getCol(), tetr.getCellState( i, j ) );
            }
        }
    }

    pb.clearCompletedRow( tetr.getRow() + tetr.getContainerSize() - 1, tetr.getRow() );
}

bool checkCollision( PlayBoard pb, Tetrimino tetr, int rowAdjustment, int colAdjustment ) {
    for ( int i = 0; i < tetr.getContainerSize(); i++ )
    {
        for ( int j = 0; j < tetr.getContainerSize(); j++ )
        {
            if ( tetr.getCellState( i, j )!= 0 && pb.getCellState( tetr.getRow() + rowAdjustment + i, tetr.getCol() + colAdjustment + i ) != 0)
                return true;
        }
    }
    return false;
}