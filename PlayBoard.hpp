#ifndef PlayBoard_hpp
#define PlayBoard_hpp
#include <vector>
#include "audio.hpp"
using namespace std;

extern const int WIDTH_BY_TILE;
extern const int HEIGHT_BY_TILE;
extern int TILE_WIDTH;

//11 types of tetromino block
enum Ttype { I_PIECE = 1, J_PIECE, L_PIECE, O_PIECE, S_PIECE, Z_PIECE, T_PIECE, UNSTABLE_PIECE, GARBAGE_PIECE, BOMB_PIECE, CLEAR };

//3 rows at the top of the board are hidden, at which new tetromino is spawned.
extern const int HIDDEN_ROW;
extern int BOARD_WIDTH;
extern int BOARD_HEIGHT;

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
         * Count completed rows from row lowerRow to row upperRow, and marks the tile that will be cleared
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