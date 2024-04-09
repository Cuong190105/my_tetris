#ifndef PlayBoard_hpp
#define PlayBoard_hpp
#include <vector>
using namespace std;

const int WIDTH_BY_TILE = 10;
const int HEIGHT_BY_TILE = 23;
const int TILE_WIDTH = 40;

//11 types of tetromino block
enum Ttype { I_PIECE = 1, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE, GARBAGE_PIECE, UNSTABLE_PIECE, BOMB_PIECE, CLEAR };

//3 rows at the top of the board are hidden, at which new tetromino is spawned.
const int HIDDEN_ROW = 3;

const int BOARD_WIDTH = WIDTH_BY_TILE * TILE_WIDTH;
const int BOARD_HEIGHT = ( HEIGHT_BY_TILE - HIDDEN_ROW ) * TILE_WIDTH;

class PlayBoard {
    private:
        
        //Width and height of play board when displayed on screen
        int w, h;

        //A matrix to store all tiles placed on the board
        vector<vector<int>> boardState;
    public:
        //Play Board Constructor
        PlayBoard();

        //Destructor
        ~PlayBoard();

        /**
         * Gets the state of a cell given its coordinate
         * \return State of the corresponding cell. -1 if the coordinate is outside the board
         */
        int getCellState( int row, int col ) const;

        //Changes the value of a cell
        void modifyCell( int row, int col, int val );
        
        /**
         * Clears all completed rows from row lowerRow to row upperRow
         * \return number of cleared rows
         */
        int completedRow( int upperRow, int lowerRow );

        //Removes all cells that are marked as cleared before.
        void deleteClearedCell();

        //Adds a blank row to the board
        void addRow( int pos );
        
        //Removes a row from the board
        void removeRow( int pos );
};

#endif