#ifndef Tetromino_hpp
#define Tetromino_hpp
#include <vector>
#include "PlayBoard.hpp"

using namespace std;
const int START_ROW = 20;
const int START_COL = 3;


class Tetromino {
    private:
        //A n*n matrix which stores the state of each tile of the tetromino (a playing piece that falls from the top of the board).
        vector<vector<int>> state;

        //Width n of the state matrix.
        int containerSize;

        //Store type of tetromino (I, L, J, etc), get value from Ttype enumerator.
        int type;

        //Store column number that container's bottom left cell is currently in.
        int currentCol;
        
        //Store row number that container's bottom left cell is currently in.
        int currentRow;

        /**
            Store current rotation state of tetromino.
            There are 4 rotation states. Every tetromino spawns with a default state called state 0.
            State 1, 2, 3 correspond to its state when rotated 90, 180 and 270 degrees clockwise from spawn state.
            \return
        */
        int rotationState;

        bool corrupted;
    public:
        //Default constructor with no arguments
        Tetromino();
        
        //Tetromino constructor
        Tetromino( int _type, int row = START_ROW, int col = START_COL );

        //Tetromino destructor
        ~Tetromino();
        
        //Returns type
        int getType() const;

        //Voids tetromino after locking it
        void voidPiece();

        //Returns containerSize
        int getContainerSize() const;

        //Returns a state's cell
        int getCellState( int row, int col ) const;

        //Returns currentCol
        int getCol() const;

        //Returns currentRow
        int getRow() const;

        //Returns rotationState
        int getRotationState() const;
        
        //Modifies the content of a state's cell
        void modifyCell( int col, int row, int value );

        //Modifies currentCol
        void updateCol( int col );

        //Modifies currentRow
        void updateRow( int row );

        //Modifies rotationState
        void updateRotationState( int newState );

        //Turns a tetromino to its giant version
        void makeItGiant();
        
        void corruptPiece();

        bool isCorrupted() const;
};

#endif