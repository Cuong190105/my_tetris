#ifndef Tetrimino_hpp
#define Tetrimino_hpp
#include <vector>
#include "PlayBoard.hpp"

using namespace std;

class Tetrimino {
    private:
        /*
        state:
            A n*n matrix which stores the state of each tile of the tetrimino (a playing piece that falls from the top of the board).
        */
        vector<vector<int>> state;

        /*
        containerSize:
            Width n of the state matrix.
        */
        int containerSize;

        /*
        type:
            Store type of tetrimino (I, L, J, etc), get value from Ttype enumerator.
        */
        int type;

        /*
        currentCol:
            Store column number that container's bottom left cell is currently in.
        */
        int currentCol;
        
        /*
        currentRow:
            Store row number that container's bottom left cell is currently in.
        */
        int currentRow;

        /*
        currentRotationState:
            Store current rotation state of tetrimino.
            There are 4 rotation states. Every tetrimino spawns with a default state called state 0.
            State 1, 2, 3 correspond to its state when rotated 90, 180 and 270 degrees clockwise from spawn state.
        */
        int currentRotationState;

    public:
        Tetrimino(int _type);

        // ~Tetrimino();
        
        int getContainerSize();

        int getCellState( int row, int col );

        int getCol();

        int getRow();

        void updateState( int col, int row, int value );

        void updateCol( int col );

        void updateRow( int row );

        int getGhostRow( PlayBoard board );

        void dropPiece( PlayBoard board, bool isHardDrop );

        void rotatePiece( PlayBoard board, bool rotateClockwise );
};

#endif