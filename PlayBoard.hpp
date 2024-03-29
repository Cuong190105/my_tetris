#ifndef PlayBoard_hpp
#define PlayBoard_hpp
#include <vector>
using namespace std;

const int WIDTH_BY_TILE = 10;
const int HEIGHT_BY_TILE = 23;
const int TILE_WIDTH = 30;

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
        int countCompletedRow( int upperRow, int lowerRow );

        //Updates board's state & statistic (score/line cleared) whenever a tetromino locks in
        void updateBoard( int upperRow = HEIGHT_BY_TILE - 1, int lowerRow = 0 );

};

#endif