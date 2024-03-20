#include "Player.hpp"
#include <cmath>
#include <iostream>
using namespace std;

enum Action {ROTATE, MOVE_LR, DROP};

Player::Player( int _level )
{
    pb = PlayBoard();
    tetr = Tetromino();
    hold = Tetromino();
    bool hold = false;
    score = 0;
    line = 0;
    level = _level;
    combo = -1;
    b2b = -1;
    lastMove = DROP;
}

Player::~Player() {}

int Player::getScore()
{
    return score;
}

int Player::getLine()
{
    return line;
}

int Player::getLevel()
{
    return level;
}

int Player::getGhostRow() const
{
    //In some cases, the first (& second) row of tetromino container is empty.
    //So It's still valid for them to stay out of bound. 
    for ( int i = 0; i <= tetr.getRow() + 2; i++ )
    {
        if ( checkCollision( tetr, - i - 1 ) ) return tetr.getRow() - i;
    }
    return -1;
}

void Player::pullNewTetromino( const vector<Tetromino> &Tqueue )
{
    tetr = Tqueue[0];
    pullMark = SDL_GetTicks();
    movesBeforeLock = 0;
    lowestRow = tetr.getRow();
}

bool Player::checkCollision( const Tetromino &tetr, int rowAdjustment, int colAdjustment ) const
{
    for ( int row = 0; row < tetr.getContainerSize(); row++ )
    {
        for ( int col = 0; col < tetr.getContainerSize(); col++ )
        {
            if ( tetr.getCellState( row, col ) != 0 && 
                pb.getCellState( tetr.getRow() + rowAdjustment + row, tetr.getCol() + colAdjustment + col ) != 0)
            {
                return true;
            }
        }
    }
    return false;
}

void Player::lockTetromino()
{
    for ( int row = 0; row < tetr.getContainerSize(); row++ )
        for ( int col = 0; col < tetr.getContainerSize(); col++ )
            if ( tetr.getCellState(row, col) != 0 )
            {
                pb.modifyCell( tetr.getRow() + row, tetr.getCol() + col, tetr.getCellState(row, col) );
            }
    int lineCleared = pb.countCompletedRow( tetr.getRow() + tetr.getContainerSize() - 1, tetr.getRow() );
    updateScore( lineCleared );
    pb.updateBoard( tetr.getRow() + tetr.getContainerSize() - 1, tetr.getRow() );
    tetr.voidPiece();
}

void Player::movePieceHorizontally( bool right )
{
    if ( !checkCollision( tetr, 0, right * 2 - 1 ) )
    {
        tetr.updateCol( tetr.getCol() + right * 2 - 1 );
        lastMove = MOVE_LR;
        movesBeforeLock++;
        lockMark = SDL_GetTicks();
    }
}

void Player::dropPiece( bool isHardDrop, bool isGravityPull )
{
    bool pieceLocked = false;
    if ( isHardDrop )
    {
        //Hard drop always locks the current piece
        pieceLocked = true;
        score += (tetr.getRow() - getGhostRow()) * 2;
        tetr.updateRow( getGhostRow() );
        lockTetromino();
        holdLock = false;
    }
    else
    {
        //Soft drop only locks the current piece if it cannot move downwards anymore. 
        if ( !checkCollision( tetr, -1 ) )
        {
            tetr.updateRow( tetr.getRow() - 1 );
            lastMove = DROP;
            score += 1 - isGravityPull;
            //Resets gravity pull timer
            pullMark = SDL_GetTicks();

            if ( tetr.getRow() < lowestRow ){
                lowestRow = tetr.getRow();
                movesBeforeLock = 0;
            if ( checkCollision( tetr, -1 ) ) lockMark = SDL_GetTicks();
            }
        }
        else lockDelayHandler();
    }
}

void Player::rotatePiece( bool rotateClockwise )
{
    //O piece doesn't need rotating
    if ( tetr.getType() == O_PIECE ) return;

    //tmp is the result of rotating this piece. I piece is declared to spawn 1 row lower,
    //so we add 1 to maintain the current row for the rotation.
    Tetromino tmp = tetr;

    //Rotate state matrix 90deg, direction depending on rotateClockwise flag
    for ( int i = 0; i < tetr.getContainerSize(); i++ )
    {
        for ( int j = 0; j < tetr.getContainerSize(); j++ )
        {
            if ( rotateClockwise )
            {
                tmp.modifyCell( tetr.getContainerSize() - j - 1, i, tetr.getCellState( i, j ) ); 
            }
            else
            {
                tmp.modifyCell( j, tetr.getContainerSize() - i - 1, tetr.getCellState( i, j ) );
            }
        }
    }

    /**
     * Stores data using for adjusting the position of the rotated piece
     * if it collides with other pieces or the board. A piece fails to rotate if all 5 adjusted positions
     * are invalid. A dataset contains 5 pairs of integers {colAdjustment, rowAdjustment}.

     * For counterclockwise rotation from state i+1 to i, take the wall kick dataset of
     * rotation from state i to i+1 then multiply all the elements with -1.
     * \return
    */
    vector<vector<int>> wallKickAdjustments;
    const int ADJUSTMENTS_COUNT = 5;

    //coeff = dirCoeff * datasetCoeff
    //dirCoeff = -1 if rotating counterclockwise, else 1
    //datasetCoeff = -1 if the circumstance noted in if-block used for choosing dataset below happens, else 1
    int coeff = rotateClockwise ? 1 : -1;

    if ( tetr.getType() != I_PIECE )
    {
        if ( ( rotateClockwise && tetr.getRotationState() < 2 ) || ( !rotateClockwise &&  tetr.getRotationState() > 0 && tetr.getRotationState() < 3 ) )
        {

            //This set is for rotation from state 0 to state 1.
            wallKickAdjustments = vector<vector<int>>
            {
                vector<int>{0, 0}, vector<int>{-1, 0},
                vector<int>{-1, 1}, vector<int>{0, -2},
                vector<int>{-1, -2}
            };
            //For rotation from state 1 to 2, multiply each coefficient with -1.
            
            //Magic number 2: 2 rotations where datasetCoeff = -1
            //1. rotation from state 1 to 2 => tetr.getRotationState() = 1, rotateClockwise = 1
            //2. rotation from state 2 to 1 => tetr.getRotationState() = 2, rotateClockwise = 0
            if ( rotateClockwise + tetr.getRotationState() == 2 ) coeff *= -1;

        }
        else
        {
            //This set is for rotating from state 2 to state 3
            wallKickAdjustments = vector<vector<int>>
            {
                vector<int>{0, 0}, vector<int>{1, 0},
                vector<int>{1, 1}, vector<int>{0, -2},
                vector<int>{1, -2}
            };
            //For rotation from state 3 to 0, multiply each coefficient with -1.
            //Some other magic numbers: 0 and 4
            if ( ( rotateClockwise + tetr.getRotationState() ) % 4 == 0 ) coeff *= -1;
        }
    }
    else
    {
        if ( ( rotateClockwise + tetr.getRotationState() ) & 1 )
        {
            //This set is for rotation from state 0 to state 1.
            //For rotation from state 2 to 3, multiply each coefficient with -1.
            wallKickAdjustments = vector<vector<int>>
            {
                vector<int>{0, 0}, vector<int>{-2, 0},
                vector<int>{1, 0}, vector<int>{2, -1},
                vector<int>{1, 2}
            };
            if ( rotateClockwise + tetr.getRotationState() == 3 ) coeff *= -1;
        }
        else
        {
            //This set is for rotating from state 1 to state 2.
            //For rotation from state 3 to 0, multiply each coefficient with -1.
            wallKickAdjustments = vector<vector<int>>
            {
                vector<int>{0, 0}, vector<int>{-1, 0},
                vector<int>{2, 0}, vector<int>{-1, 2},
                vector<int>{2, -1}
            };

            if ( ( rotateClockwise + tetr.getRotationState() ) % 4 == 0 ) coeff *= -1;
        }
    }



    for ( int i = 0; i < ADJUSTMENTS_COUNT; i++ )
    {
        int colAdjustments = wallKickAdjustments[i][0] * coeff;
        int rowAdjustments = wallKickAdjustments[i][1] * coeff;
        if ( !checkCollision( tmp, rowAdjustments, colAdjustments ) )
        {
            tetr.updateRotationState( tetr.getRotationState() + ( rotateClockwise ? 1 : -1 ) );
            if ( tetr.getRotationState() == 4 ) tetr.updateRotationState(0);
            else if ( tetr.getRotationState() == -1 ) tetr.updateRotationState(3);
            for ( int i = 0; i < tetr.getContainerSize(); i++ )
            {
                for ( int j = 0; j < tetr.getContainerSize(); j++ )
                {
                    tetr.modifyCell( i, j, tmp.getCellState( i, j ) );
                }
            }
            tetr.updateRow( tetr.getRow() + rowAdjustments );
            tetr.updateCol( tetr.getCol() + colAdjustments );
            lastMove = ROTATE;
            lockMark = SDL_GetTicks();
            movesBeforeLock++;
            return;
        }
    }
}

void Player::lockDelayHandler()
{
    if ( movesBeforeLock >= 15 || SDL_GetTicks() - lockMark > 500 ) dropPiece( true );
}

int Player::pullInterval()
{
    //pullInterval = g * 1000
    //g: Gravity - numver of row pulled per second
    return pow( ( 0.8 - (level - 1) * 0.007 ), level - 1 ) * 1000;
}

void Player::gravityPull()
{
    if ( checkCollision( tetr, -1 ) )
    {
        lockDelayHandler();
    }
    else if ( ( SDL_GetTicks() - pullMark ) > pullInterval() )
    {
        dropPiece( false, true );
        pullMark = SDL_GetTicks();
    }
}

enum TspinState { NO_SPIN, MINI_TSPIN, TSPIN };

int Player::tspinCheck()
{
    if ( tetr.getType() == T_PIECE && lastMove == ROTATE )
    {
        int emptyCorner = 0;
        int emptyCorner_row = 0;
        int emptyCorner_col = 0;
        for ( int i = 0; i < 3; i += 2 )
        {
            for ( int j = 0; j < 3; j += 2 )
            {
                if ( pb.getCellState( tetr.getRow() + i, tetr.getCol() + j ) == 0 ) emptyCorner++;
                if ( emptyCorner == 2 ) return NO_SPIN;
                else
                {
                    emptyCorner_row = i;
                    emptyCorner_col = j;
                }
            }
        }
        if ( emptyCorner == 0 )
        {
            return TSPIN;
        }
        else
        {
            if ( tetr.getCellState( emptyCorner_row, 1) && tetr.getCellState( 1, emptyCorner_col ) )
            {
                return TSPIN;
            }
            else return MINI_TSPIN;
        }
    }
    return NO_SPIN;
}

void Player::updateScore( int lineCleared, int delta )
{
    int tspinState = tspinCheck();
    switch( tspinState )
    {
        case TSPIN:
            delta += (lineCleared + 1) * 400;
            break;
        case MINI_TSPIN:
            delta += pow( 2, lineCleared ) * 200;
            break;
        case NO_SPIN:
            switch( lineCleared )
            {
                case 4:
                    delta += 300;
                case 3:
                    delta += 200;
                case 2:
                    delta += 200;
                case 1:
                    delta += 100;
                    if (lineCleared < 4) b2b = -1;
            }
    }
    if ( lineCleared == 0 )
    {
        combo = -1;
    }
    else
    {
        combo++;
        if ( tspinState != NO_SPIN || lineCleared == 4 ) b2b ++;
    }
    delta = delta * ( 2 + ( b2b > 0 ) ) / 2;
    if ( combo > 0 ) delta += combo * 50;
    score += delta * level;
    line += lineCleared;
    level = line / 10 + 1;
}

void Player::swapHoldPiece()
{
    if ( !holdLock )
    {
        int tmp = hold.getType();
        hold = Tetromino( tetr.getType() );
        tetr = Tetromino( tmp );
        holdLock = true;
        movesBeforeLock = 0;
        lowestRow = tetr.getRow();
    }
}

