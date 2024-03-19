#ifndef PlayBoard_hpp
#define PlayBoard_hpp
#include <vector>
using namespace std;

const int WIDTH_BY_TILE = 10;
const int HEIGHT_BY_TILE = 23;
const int TILE_WIDTH = 30;

//3 rows at the top of the board are hidden, at which new tetrimino is spawned.
const int HIDDEN_ROW = 3;

class PlayBoard {
    private:
        
        //Width and height of play board when displayed on screen
        int w, h;

        //A matrix to store all tiles placed on the board
        vector<vector<int>> boardState;

        //Score earned, lines cleared and current level on this board
        //Current level: Indicates tetrimino's falling speed. Normally, level get +1 for every 10 lines cleared.
        int score, line, level;
    public:
        //Play Board Constructor
        PlayBoard( int _level = 1 );

        //Destructor
        ~PlayBoard();

        //Gets the width of the board
        int getWidth() const;

        //Gets the height of the board
        int getHeight() const;

        //Gets current board's score
        int getScore() const;

        //Gets current board's lines cleared
        int getLines() const;

        //Gets current board's level
        int getLevel() const;
        
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
        int clearCompletedRow( int upperRow, int lowerRow );

        //Updates board's state & statistic (score/line cleared) whenever a tetrimino locks in
        void updateBoard( int upperRow = HEIGHT_BY_TILE - 1, int lowerRow = 0 );

};

#endif