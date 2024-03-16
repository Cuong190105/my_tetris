#ifndef Tetrimino_hpp
#define Tetrimino_hpp
#include <vector>
#include "PlayBoard.hpp"

using namespace std;
const int START_ROW = 20;
const int START_COL = 3;

class Tetrimino {
    private:
        //A n*n matrix which stores the state of each tile of the tetrimino (a playing piece that falls from the top of the board).
        vector<vector<int>> state;

        //Width n of the state matrix.
        int containerSize;

        //Store type of tetrimino (I, L, J, etc), get value from Ttype enumerator.
        int type;

        //Store column number that container's bottom left cell is currently in.
        int currentCol;
        
        //Store row number that container's bottom left cell is currently in.
        int currentRow;

        /**
            Store current rotation state of tetrimino.
            There are 4 rotation states. Every tetrimino spawns with a default state called state 0.
            State 1, 2, 3 correspond to its state when rotated 90, 180 and 270 degrees clockwise from spawn state.
            \return
        */
        int currentRotationState;

    public:
        //Tetrimino constructor
        Tetrimino( int _type, int row = START_ROW, int col = START_COL );

        //Tetrimino destructor
        ~Tetrimino();
        
        //Return containerSize
        int getContainerSize();

        //Return a state's cell
        int getCellState( int row, int col );

        //Return currentCol
        int getCol();

        //Return currentRow
        int getRow();
        
        //Modify the content of a state's cell
        void updateState( int col, int row, int value );

        //Modify currentCol
        void updateCol( int col );

        //Modify currentRow
        void updateRow( int row );

        /**
         * Return the row of the ghost of this tetrimino
         * Ghost: A piece resembles current piece displayed on top of current stack
         * that indicates where the current piece will land on if it continues to 
         * fall down until colliding with the stack without moving horizontally.
         * \return 
         */
        int getGhostRow( PlayBoard board );

        /**
         * Checks if this tetrimino collides with any other element (border, another active cell) of the playfield
         * \param board The playfield this tetrimino is currently in.
         * \param rowAdjustment \param colAdjustment Adjusting tetrimino's position for specific purposes (checking valid movement or rotation, etc.)
        */
        bool checkCollision( PlayBoard board, int rowAdjustment = 0, int colAdjustment = 0 );

        /**
         * Control horizontal movement of the piece.
         * \return false if the movement is invalid, else return true.
         */
        void movePieceHorizontally( PlayBoard board, bool right );

        /**
         * Control dropping movement of the piece
         * Soft drop: The piece drops one row at a time
         * Hard drop: The piece drops all the way down to the top of the stack
         */
        void dropPiece( PlayBoard board, bool isHardDrop );

        //Control rotation of the piece
        void rotatePiece( PlayBoard board, bool rotateClockwise );
};

#endif