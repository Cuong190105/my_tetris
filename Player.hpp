#ifndef Player_hpp
#define Player_hpp
#include "Tetromino.hpp"
#include "Texture.hpp"
#include "audio.hpp"
#include <SDL.h>

enum KeyboardFunction { K_LEFT, K_RIGHT, K_DOWN };

class Player
{
    private:
        //Position and dimenstions of this player's playfield
        int x, y;

        PlayBoard pb;
        Tetromino tetr;
        Tetromino hold;
        bool holdLock;
        int score, line, level;
        int mode;

        //Turn counter in mystery mode (A turn is the time a piece is spawned then locked onto the stack)
        int turn;

        //Marks the time a piece locked in for delaying spawning a new piece
        Uint32 delaySpawnMark;
        
        //Charges the rotation command during the delay spawning new piece. Hold rotate button (Up, x, z)
        //to charge. If the button is held until new piece is created, it will be spawned in other direction like being rotated before.
        int irsCharge;

        //Marks the last time soft dropping current tetromino
        Uint32 pullMark;

        //Stores the interval between each gravity's pull        
        float pullInterval;

        pair<int, int> keyRepeatState[3];
        /**
         * Marks the triggering point of autolock timer.
         * Timer resets when doing a move other than dropping it lower than the lowest reached row
         * Timer is cancelled when falling below the lowest reached point.
         */
        Uint32 lockMark;

        int lockDelay;

        //Counts the number of moves made after triggering autolock timer. After reaching 15, the timer expires immediately.
        //Counter resets when the timer is cancelled or a new piece is drawn (from the queue or hold box).
        int movesBeforeLock, lowestRow;

        //Streak (Multiple(greater than one) consecutive line clears) of clearing line
        int combo;

        //Difficult Line clear streak: Tspin or Tetris (Quad line clear)
        //Line clear don't have to be consecutive (can stack piece without clearing line between 2 difficult line clears,
        //but easy line clears are not allowed to do so in order not to break back-to-back streak)
        //Tspins which don't clear any line won't be counted in b2b streak, but it don't break the streak.
        int b2b;

        //Mark the time triggering bonus. Used for making animated indicators.
        //Idx 0: Combo, 1: Tspin/Tetris(either can happen at a time) + B2B (only Tspins/Tetrises are counted for b2b), 3: All Clear
        Uint32 bonusMark[3];
        int bonus;

        //Stores last move (Rotate, move left/right, or non-locking drop)
        int lastMove;

        //Stores current event in Mystery mode (-1 for no event)
        int mysteryEvent;

        //Marks start time of current event
        Uint32 mysteryMark;

        //Stores player's status
        bool gameOver;
    public:
        Player( int _level, int _mode, int _x, int _y );
        ~Player();

        //Returns this player's score
        int getScore() const;

        //Returns this player's line cleared
        int getLine() const;

        //Returns this player's level
        int getLevel() const;

        void setLevel( int _level );

        int getMode() const;
        
        int getMysteryEvent() const;

        int getMysteryMark() const;
        
        int getSpawnDelayMark() const;

        void setLockDelay();

        //Adjusts all time marks after resume playing
        void setTimeMark( Uint32 pauseMark );

        void terminateGame();

        bool isGameOver();

        void setX( int _x );
        int getX() const;

        void setY( int _y );
        int getY() const;

        /**
         * Returns the row of the ghost of current tetromino on playfield
         * Ghost: A piece resembles current piece displayed on top of the stack
         * that indicates where the current piece will land on if it continues to 
         * fall down until colliding with the stack without moving horizontally.
         * \return 
         */
        int getGhostRow() const;

        //Pulls new tetromino from a shared queue (useful in multiplayer mode)
        void pullNewTetromino( const vector<Tetromino>& Tqueue );

        /**
         * Checks if this tetromino collides with any other element (border, another active cell) of the playfield
         * \param board The playfield this tetromino is currently in.
         * \param rowAdjustment \param colAdjustment Adjusting tetromino's position for specific purposes (checking valid movement or rotation, etc.)
        */
        bool checkCollision( const Tetromino &tetr , int rowAdjustment = 0, int colAdjustment = 0 ) const;

        //Handles tetromino's lock
        void lockTetromino();

        /**
         * Controls horizontal movement of the piece.
         * \return false if the movement is invalid, else return true.
         */
        void movePieceHorizontally( bool right );

        /**
         * Controls dropping movement of the piece
         * Soft drop: The piece drops one row at a time
         * Hard drop: The piece drops all the way down to the top of the stack
         * \return true if the drop locks the piece on the stack, else false.
         */
        void dropPiece( bool isHardDrop, bool isGravityPull = false );

        //Controls rotation of the piece
        void rotatePiece( bool rotateClockwise );
        
        /**
         * Handles piece autolocking after landing on the stack.
         * After softdropping on the stack, the piece triggers an autolock timer independent from falling speed, 
         * which will later lock the piece on expiration. Valid L/R movement or rotation will reset the timer, and 
         * 
         */
        void lockDelayHandler();

        //Pulls the tetromino down (simulating gravity)
        void gravityPull();

        /**
         * Checks for valid T-spin
         * Tspin: Rotates a T-piece into a tight space that occupies at least 3 corners of the T-piece's container;
         * Illustration ( o: T-piece's tile, x: tile placed before, -: tile placed or empty cell):
         *  x - -   |   x o -   |   x o -
         *  o o o   |   o o -   |   o o o
         *  x o x   |   x o x   |   x - x
         * Requirements:
         * . Must be T-piece
         * . Last move must be rotation
         * . At least 3/4 corner occupied by the stack
         * . Mini Tspin additional requirement: Only 1 front corner is occupied, gets less reward than a "proper" Tspin
         */
        int tspinCheck();

        void updateScore( int lineCleared, int delta = 0 );

        void swapHoldPiece();

        void handleMysteryEvents( vector<Tetromino> &Tqueue );

        //===================================GRAPHICS================================
        void displayBoard();

        void displayPreviewTetromino( int _x, int _y, const Tetromino &Ptetr );

        void displayCurrentTetromino();

        void displayHeldTetromino();

        void displayTetrominoQueue( vector<Tetromino> &Tqueue, int previewPieces = 5, int queuePosition = 0 );

        // Draw pieces on the board;
        void displayBoardCell();

        void displayBonus();

        void renderGameOver();

        void ingameProgress( const vector<Tetromino> &Tqueue, int &queuePosition, int &scene );

        void handlingKeyPress( bool &gameOver, int &scene );
};

enum bonusType { MINI = 1, T_SPIN = 2, B2B = 4, ALLCLEAR = 8, TETRIS = 16 };
enum soloMode { CLASSIC, SPRINT, BLITZ, MASTER, MYSTERY };
enum multiMode { SCORE = MYSTERY + 1, ATTACK, MYSTERY_ATTACK };
enum mysteryType { UPSIDE_DOWN, UNSTABLE, BOMB, GIANT, ADD_GARBAGE, CORRUPTED, HORIZONTAL_SHIFT, EVENT_NUMBER };
enum modType { LEVEL, LINECAP, TIME, ACTIVATE_MYSTERY };


#endif