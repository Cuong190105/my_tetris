#include "Player.hpp"

#include <cmath>
#include <iostream>
using namespace std;

enum Action {ROTATE, MOVE_LR, DROP};

Player::Player( int _level, int _mode, int _x, int _y, bool isActive )
{
    pb = PlayBoard();
    tetr = Tetromino();
    hold = Tetromino();
    holdLock = false;
    score = 0;
    line = 0;
    combo = -1;
    b2b = -1;
    lastMove = DROP;
    movesBeforeLock = 0;
    lowestRow = 21;
    x = _x; y = _y;
    spd_x = 0; spd_y = 0;
    lockDelay = 500;
    mode = _mode;
    if (mode == MASTER) pullInterval = 1;
    gameOver = false;
    setLevel(_level);
    bonus = 0;
    delaySpawnMark = SDL_GetTicks();
    turn = 0;
    mysteryEvent = -1;
    activePlayer = isActive;
}

Player::~Player() {}

int Player::getScore() const { return score; }
int Player::getLine() const { return line; }

int Player::getLevel() const { return level; }
void Player::setLevel( int _level )
{
    level = _level;
    if (mode != MASTER && level < 20) pullInterval = pow((0.8 - (level - 1) * 0.007),level - 1) * 1000;
    else setLockDelay();
}

void Player::setLockDelay()
{
    if ( mode == CLASSIC || mode == SCORE )
    lockDelay = 500 - (level > 19 ? min(level, 25) - 19 : 0) * 50 - (level > 25 ? level - 25 : 0) * 10;
    else if ( mode == MASTER )
    lockDelay = 485 - min(level, 20) * 15 - (level > 20 ? level - 20 : 0) * 10;
}

void Player::setTimeMark( Uint32 pauseMark )
{
    Uint32 now = SDL_GetTicks();
    pullMark += now - pauseMark;
    lockMark += now - pauseMark;
    bonusMark[0] += now - pauseMark;
    bonusMark[1] += now - pauseMark;
    bonusMark[2] += now - pauseMark;
}

int Player::getMode() const { return mode; }
void Player::terminateGame()
{ 
    gameOver = true;
    if (activePlayer)
    {
        if (!isHost) client.sendToServer( to_string(client.getPosition()) + "term" );
        else
        {
            for (int i = 0; i < server.getClientNum(); i++)
            {
                server.makeMsg( "0term", i );
            }
            server.sendToClient();
        }
    }
}
bool Player::isGameOver() { return gameOver; }

int Player::getMysteryEvent() const { return mysteryEvent; }
int Player::getMysteryMark() const { return mysteryMark; }

int Player::getSpawnDelayMark() const { return delaySpawnMark; }

void Player::setX( int _x ) { x = _x; }
int Player::getX() const{ return x; }

void Player::setY( int _y ) { y = _y; }
int Player::getY() const { return y; }

int Player::getGhostRow() const
{
    //In some cases, the first (& second) row of tetromino container is empty.
    //So It's still valid for them to stay out of bound.
    if ( tetr.getType() == BOMB_PIECE ) return tetr.getRow();
    for ( int i = 0; i <= tetr.getRow() + 2; i++ )
    {
        if ( checkCollision( tetr, - i - 1 ) ) return tetr.getRow() - i;
    }
    return -1;
}

void Player::pullNewTetromino( const vector<Tetromino> &Tqueue, int &pos )
{
    if ( !activePlayer && tetr.getType() != 0 ) dropPiece( true );
    if ( mode >= SCORE && activePlayer )
    {
        if ( isHost )
        {
            for ( int i = 0; i < server.getClientNum(); i++ )
            {
                server.makeMsg( "0pull", i );
            }
            server.sendToClient();
        }
        else
        {
            client.sendToServer( to_string(client.getPosition()) + "pull" );
        }
    }
    tetr = Tqueue[pos++];
    pullMark = SDL_GetTicks();
    movesBeforeLock = 0;
    lowestRow = tetr.getRow();
    if ( irsCharge == 1) rotatePiece( true );
    else if ( irsCharge == -1) rotatePiece( false );
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
    turn ++;
    int additionDelay = 0;
    playSfx( LOCK );
    holdLock = false;
    spd_y = 3;
    if ( tetr.getType() == BOMB_PIECE )
    {
        int cnt = 0;
        for ( int row = max(0, tetr.getRow() - 3); row < min( HEIGHT_BY_TILE, tetr.getRow() + 4); row++ )
        {
            for ( int col = max(0, tetr.getCol() - 3); col < min( WIDTH_BY_TILE, tetr.getCol() + 4); col++ )
            {
                if ( pb.getCellState( row, col ) > 0 ) cnt ++;
                pb.modifyCell( row, col, CLEAR );
            }
        }
        score += 20 * cnt;
    }
    else
    {
        for ( int row = 0; row < tetr.getContainerSize(); row++ )
            for ( int col = 0; col < tetr.getContainerSize(); col++ )
                if ( tetr.getCellState(row, col) != 0 )
                {
                    pb.modifyCell( tetr.getRow() + row, tetr.getCol() + col, tetr.getCellState(row, col) );
                }
        if ( tetr.getCellState( 1, 1 ) != UNSTABLE_PIECE && tetr.getCellState( 2, 2 ) != UNSTABLE_PIECE )
        {
            int lineCleared = pb.completedRow( tetr.getRow() + tetr.getContainerSize() - 1, tetr.getRow() );
            bool allClear = true;
            for ( int row = 0; row < HEIGHT_BY_TILE; row++ )
            {
                for ( int col = 0; col < WIDTH_BY_TILE; col++ )
                    if ( pb.getCellState( row, col ) > 0 ) { allClear = false; break; }
                if ( !allClear ) break;
            }
            if ( allClear ) bonus |= ALLCLEAR;
            updateScore( lineCleared );
        }
        else additionDelay = 150;
    }
    tetr.voidPiece();
    delaySpawnMark = SDL_GetTicks() + additionDelay;
    lockMark = SDL_GetTicks();
}

void Player::movePieceHorizontally( bool right )
{
    if ( tetr.getType() == BOMB_PIECE && (( right && tetr.getCol() < WIDTH_BY_TILE - 1) || (!right && tetr.getCol() > 0)) || !checkCollision( tetr, 0, right * 2 - 1 ) )
    {
        tetr.updateCol( tetr.getCol() + right * 2 - 1 );
        lastMove = MOVE_LR;
        movesBeforeLock++;
        lockMark = SDL_GetTicks();
        playSfx( MOVE );
        if ( mode >= SCORE && activePlayer )
        {
            if (!isHost) client.sendToServer( to_string(client.getPosition()) + (right ? "rmov" : "lmov") );
            else
            {
                for ( int i = 0; i < server.getClientNum(); i++ )
                {
                    server.makeMsg( to_string(0) + (right ? "rmov" : "lmov"), i );
                }
                server.sendToClient();
            } 
        }
    }
    else
    {
        spd_x = 3 * (2 * right - 1);
    }
}

void Player::dropPiece( bool isHardDrop, bool isGravityPull )
{
    if ( isHardDrop )
    {
        //Hard drop always locks the current piece
        score += (tetr.getRow() - getGhostRow()) * 2;
        if ( getGhostRow() != tetr.getRow() ) lastMove = DROP;
        tetr.updateRow( getGhostRow() );
        lockTetromino();
        playSfx( HARDDROP );
        if ( mode >= SCORE && activePlayer )
        {
            if (!isHost) client.sendToServer( to_string(client.getPosition()) + "hdrp" );
            else
            {
                for ( int i = 0; i < server.getClientNum(); i++ )
                {
                    server.makeMsg( "0hdrp", i );
                }
                server.sendToClient();
            } 
        }
    }
    else
    {
        //Soft drop only locks the current piece if it cannot move downwards anymore.
        if ( (tetr.getType() == BOMB_PIECE && tetr.getRow() > 0 ) || !checkCollision( tetr, -1 ) )
        {
            tetr.updateRow( tetr.getRow() - 1 );
            lastMove = DROP;
            score += 1 - isGravityPull;
            //Resets gravity pull timer
            pullMark = SDL_GetTicks();

            if ( tetr.getRow() < lowestRow )
            {
                lowestRow = tetr.getRow();
                movesBeforeLock = 0;
                if ( checkCollision( tetr, -1 ) ) lockMark = SDL_GetTicks();
            }
            if ( !isGravityPull)
            {
                playSfx( MOVE );
                if ( mode >= SCORE && activePlayer )
                {
                    if (!isHost) client.sendToServer( to_string(client.getPosition()) + "sdrp" );
                    else
                    {
                        for ( int i = 0; i < server.getClientNum(); i++ )
                        {
                            server.makeMsg( "0sdrp", i );
                        }
                        server.sendToClient();
                    } 
                }
            }
        }
        else lockDelayHandler();
    }
}

void Player::rotatePiece( bool rotateClockwise )
{
    //tmp is the result of rotating this piece.
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
        int colAdjustments = wallKickAdjustments[i][0] * coeff * ( 1 + ( tetr.getContainerSize() > 4 ));
        int rowAdjustments = wallKickAdjustments[i][1] * coeff * ( 1 + ( tetr.getContainerSize() > 4 ));
        if ( !checkCollision( tmp, rowAdjustments, colAdjustments ) )
        {
            tetr.updateRotationState( tetr.getRotationState() + ( rotateClockwise ? 1 : -1 ) );
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
            if ( mode >= SCORE && activePlayer )
            {
                if (!isHost) client.sendToServer( to_string(client.getPosition()) + (rotateClockwise ? "rrot" : "lrot") );
                else
                {
                    for ( int i = 0; i < server.getClientNum(); i++ )
                    {
                        server.makeMsg( to_string(0) + (rotateClockwise ? "rrot" : "lrot"), i );
                    }
                    server.sendToClient();
                } 
            }
            return;
        }
    }
}

void Player::lockDelayHandler()
{
    if ( movesBeforeLock >= 15 || SDL_GetTicks() - lockMark > lockDelay ) lockTetromino();
}

void Player::gravityPull()
{
    bool change = false;
    if ( tetr.getType() != BOMB_PIECE && checkCollision( tetr, -1 ) )
    {
        if ( !activePlayer ) {lockTetromino(); change = true;}
        else lockDelayHandler();
    }
    else
    {
        if ( activePlayer && ((tetr.getType() == I_PIECE && tetr.getRow() == START_ROW - 1) || tetr.getRow() == START_ROW) ) 
            {dropPiece( false, true ); change = true;}
        int rowDrop = 10 / pullInterval + 1;
        int newPullInterval = pullInterval * rowDrop;
        if ( !activePlayer || ( SDL_GetTicks() - pullMark ) > newPullInterval )
        {
            change = true;
            for ( int i = 0; i < rowDrop; i++) dropPiece( false, true );
            pullMark = SDL_GetTicks();
        }
    }
    if ( change && mode >= SCORE && activePlayer )
    {
        if (!isHost) client.sendToServer( to_string(client.getPosition()) + "gpul" );
        else
        {
            for ( int i = 0; i < server.getClientNum(); i++ )
            {
                server.makeMsg( "0gpul", i );
            }
            server.sendToClient();
        } 
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
                return MINI_TSPIN;
            }
            else return TSPIN;
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
            bonus |= T_SPIN | B2B;
            bonusMark[1] = SDL_GetTicks();
            break;
        case MINI_TSPIN:
            delta += pow( 2, lineCleared ) * 200;
            bonus |= MINI | T_SPIN | B2B;
            bonusMark[1] = SDL_GetTicks();
            break;
        case NO_SPIN:
            switch( lineCleared )
            {
                case 4:
                    bonus |= TETRIS | B2B;
                    delta += 800;
                    playSfx( TETRIS_BONUS );
                    bonusMark[1] = SDL_GetTicks();
                    break;
                case 3:
                    delta += 500;
                    b2b = -1;
                    break;
                case 2:
                    delta += 300;
                    b2b = -1;
                    break;
                case 1:
                    delta += 100;
                    b2b = -1;
                    break;
                default:
                    delta += 200 * lineCleared;
                    if (lineCleared < 4 && lineCleared > 0) b2b = -1;
                    break;
            }
    }
    if ( lineCleared == 0 )
    {
        combo = -1;
    }
    else
    {
        combo++;
        playSfx( LINE_CLEAR );
        if ( bonus & ALLCLEAR ) 
        {
            switch ( lineCleared )
            {
                case 4:
                    delta += 2000;
                    if ( b2b > 0 ) delta += 1200;
                    break;
                case 3:
                    delta += 1800;
                    break;
                case 2:
                    delta += 1200;
                    break;
                case 1:
                    delta += 800;
                    break;
                default:
                    delta += 1000;
            }
        }
        if ( tspinState != NO_SPIN && lineCleared ) bonus |= B2B;
        if (combo > 0) bonusMark[0] = SDL_GetTicks();
        if ( tspinState != NO_SPIN || lineCleared >= 4 ) b2b ++;
    }
    if ( (b2b > 0 && lineCleared) || tspinState != NO_SPIN || combo > 0 ) playSfx( BONUS_POINT );
    delta = delta * ( 2 + ( b2b > 0 ) ) / 2;
    if ( combo > 0 ) delta += combo * 50;
    score += delta * level;
    line += lineCleared;
}

void Player::handleMysteryEvents( vector<Tetromino> &Tqueue )
{
    static int tmp = 0;
    static bool eventCreated = true;
    const int TURNS_PER_EVENT = 30;
    if ( turn == TURNS_PER_EVENT )
    {
        if ( eventCreated )
        {
            mysteryEvent = -1;
            delaySpawnMark = SDL_GetTicks() + 1000;
            eventCreated = false;
            mysteryMark = SDL_GetTicks();
        }
        else if ( SDL_GetTicks() - mysteryMark > 500 )
        {
            mysteryEvent = rand() % EVENT_NUMBER;
            tmp = 0;
            turn = 0;
        }
    }
    switch( mysteryEvent )
    {
        case UPSIDE_DOWN:
            if (!eventCreated) {eventCreated = true; delaySpawnMark = SDL_GetTicks() + 100;}
            break;

        case UNSTABLE:
            if (!eventCreated)
            {
                tmp = rand() % 5 + 1;
                for ( int i = 0; i < tmp; i++ )
                {
                    for ( int row = 0; row < Tqueue[i].getContainerSize(); row++ )
                        for ( int col = 0; col < Tqueue[i].getContainerSize(); col++ )
                        {
                            if (Tqueue[i].getCellState(row, col) != 0) Tqueue[i].modifyCell( row, col, UNSTABLE_PIECE );
                        }
                }
                eventCreated = true;
            }
            break;
        case BOMB:
            if ( !eventCreated )
            {
                Tqueue.insert(Tqueue.begin(), Tetromino( BOMB_PIECE ));
                eventCreated = true;
                delaySpawnMark = SDL_GetTicks() + 100;
            }
            break;
        case GIANT:
            if ( !eventCreated )
            {
                Tqueue[0].makeItGiant();
                eventCreated = true;
                delaySpawnMark = SDL_GetTicks() + 100;
            }
            break;
        case ADD_GARBAGE:
            if ( !eventCreated )
            {
                tmp = rand() % 7 + 1;
                eventCreated = true;
            }
            else if ( tmp > 0 && SDL_GetTicks() - mysteryMark > 1000 / 9 )
            {
                int hole = rand() % WIDTH_BY_TILE;
                pb.addRow( 0 );
                for ( int j = 0; j < WIDTH_BY_TILE; j++ )
                {
                    if ( j == hole ) continue;
                    pb.modifyCell( 0, j, GARBAGE_PIECE );
                }
                mysteryMark = SDL_GetTicks();
                tmp --;
                if (tmp != 0) delaySpawnMark = SDL_GetTicks() + 1000 / 8;
            }
            break;
        case CORRUPTED:
        {
            if ( !eventCreated )
            {
                tmp = rand() % 16 + 10;
                eventCreated = true;
            }
            else if (tmp > 0) for (int i = 0; i < Tqueue.size() && tmp > 0; i++)
            {
                if ( !Tqueue[i].isCorrupted() ) { Tqueue[i].corruptPiece(); tmp --; }
            }
            break;
        }
        case HORIZONTAL_SHIFT:
        {
            static int upperRow, lowerRow;
            if ( !eventCreated )
            {
                tmp = rand() % 8 + 5;
                int stackHeight = HEIGHT_BY_TILE;
                for (int i = HEIGHT_BY_TILE - 1; i > -1; i--)
                {
                    for (int j = 0; j < WIDTH_BY_TILE; j++)
                    {
                        if ( pb.getCellState(i, j) != 0 )
                        {
                            stackHeight = i;
                            break;
                        }
                    }
                    if ( stackHeight < HEIGHT_BY_TILE ) break;
                }
                lowerRow = rand() % (stackHeight + 1);
                upperRow = rand() % (stackHeight + 1);
                if ( upperRow < lowerRow ) swap(lowerRow, upperRow);
                eventCreated = true;
            }
            else if ( tmp > 0 && SDL_GetTicks() - mysteryMark > 1000 / 9 )
            {
                for (int j = lowerRow; j <= upperRow; j++)
                {
                    int popped = pb.getCellState( j, 0 );
                    for (int k = 0; k < WIDTH_BY_TILE - 1; k++)
                    {
                        pb.modifyCell( j, k, pb.getCellState( j, k + 1 ) );
                    }
                    pb.modifyCell( j, WIDTH_BY_TILE - 1, popped );
                }
                mysteryMark = SDL_GetTicks();
                tmp --;
                if (tmp != 0) delaySpawnMark = SDL_GetTicks() + 1000 / 8;
            }
        }
            break;
        default:
            break;
    }

    static bool playingUnstablePiece = false;
    if ( tetr.getCellState( 1, 1 ) == UNSTABLE_PIECE || tetr.getCellState( 2, 2 ) == UNSTABLE_PIECE ) playingUnstablePiece = true;
    if ( playingUnstablePiece && tetr.getType() == 0 )
    {
        Uint32 nowMark = SDL_GetTicks();
        bool pulled = false;
        for (int col = 0; col < WIDTH_BY_TILE; col++)
        {
            for (int row = 0; row < HEIGHT_BY_TILE; row++)
            {
                if ( pb.getCellState( row, col ) == UNSTABLE_PIECE )
                {
                    int lastUnstableInBlock = row;
                    for ( ; lastUnstableInBlock > - 1; lastUnstableInBlock-- )
                    {
                        if ( pb.getCellState( lastUnstableInBlock - 1, col) <= 0) break;
                        else pb.modifyCell( lastUnstableInBlock - 1, col, UNSTABLE_PIECE );
                    }
                    if ( lastUnstableInBlock == 0 ) {for ( int i = 0; i <= row; i++ ) pb.modifyCell( i, col, GARBAGE_PIECE );}
                    else if ( nowMark - lockMark > 50)
                    {
                        pulled = true;
                        for (int i = lastUnstableInBlock; i <= row; i++ )
                        {
                            pb.modifyCell( i - 1, col, UNSTABLE_PIECE );
                            pb.modifyCell( i, col, 0 );
                        }
                    }
                }
            }
        }
        if ( pulled )
        {
            delaySpawnMark = nowMark + 300;
            lockMark = nowMark;
        }
        else if ( nowMark - lockMark > 50 )
        {
            int clearedRow = pb.completedRow( HEIGHT_BY_TILE - 1, 0 );
            updateScore( clearedRow );
            playingUnstablePiece = false;
            delaySpawnMark = SDL_GetTicks() + 50;
        }
    }
}

void Player::swapHoldPiece()
{
    if ( !holdLock )
    {
        if ( mode >= SCORE && activePlayer )
        {
            if (!isHost) client.sendToServer( to_string(client.getPosition()) + "swap" );
            else
            {
                for ( int i = 0; i < server.getClientNum(); i++ )
                {
                    server.makeMsg( "0swap", i );
                }
                server.sendToClient();
            } 
        }
        Tetromino tmp = hold;
        hold = tetr;
        hold.updateCol( START_COL - (hold.getContainerSize() > 4 ? hold.getContainerSize() / 2 - 1 : 0) );
        hold.updateRow( START_ROW - (hold.getType() == I_PIECE) - (hold.getContainerSize() > 4 ? hold.getContainerSize() / 2 : 0) );
        while ( hold.getRotationState() != 0 )
        {
            for ( int i = 0; i < hold.getContainerSize(); i++ )
            {
                for ( int j = 0; j < hold.getContainerSize(); j++ )
                {
                    hold.modifyCell( tetr.getContainerSize() - j - 1, i, tetr.getCellState( i, j ) ); 
                }
            }
            tetr = hold;
            hold.updateRotationState( hold.getRotationState() + 1 );
        } 
        tetr = tmp;
        holdLock = true;
        movesBeforeLock = 0;
        lowestRow = tetr.getRow();
        playSfx( MOVE );
    }
}

void Player::displayBoard()
{
    x += spd_x; y += spd_y;
    if ( abs(x - (WINDOW_WIDTH - BOARD_WIDTH) / 2) > 6)
    {
        x = (WINDOW_WIDTH - BOARD_WIDTH) / 2 + 6 * (2 * (x > (WINDOW_WIDTH - BOARD_WIDTH) / 2) - 1);
    }
    if ( y - (WINDOW_HEIGHT - BOARD_HEIGHT) / 2 > 6)
    {
        y = (WINDOW_HEIGHT - BOARD_HEIGHT) / 2 + 6;
    }
    if ( x > (WINDOW_WIDTH - BOARD_WIDTH) / 2 ) spd_x --;
    else if ( x < (WINDOW_WIDTH - BOARD_WIDTH) / 2 ) spd_x ++;
    else spd_x = 0;
    if ( y != (WINDOW_HEIGHT - BOARD_HEIGHT) / 2 ) spd_y --;
    else spd_y = 0;


    //Draw board background color
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0xFF );
    SDL_Rect board { x, y, BOARD_WIDTH, BOARD_HEIGHT };
    SDL_RenderFillRect( renderer, &board );

    //Draw board gridlines
    SDL_SetRenderDrawColor( renderer, 0x11, 0x11, 0x11, 0xFF );
    for( int i = 1; i < WIDTH_BY_TILE; i++ )
    {
        SDL_RenderDrawLine( renderer, x + TILE_WIDTH * i, y, x + TILE_WIDTH * i, y + BOARD_HEIGHT );
    }
    for( int i = 1; i < HEIGHT_BY_TILE - HIDDEN_ROW; i++ )
    {
        SDL_RenderDrawLine( renderer, x, y + TILE_WIDTH * i, x + BOARD_WIDTH, y  + TILE_WIDTH * i );
    }

    //Draw board borders
    SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    for ( int i = -1; i < 2; i++ )
    {
        //Explain: 
        //Use loop to draw many lines next to each other to create thick border
        //Add abs(i) to round line ends

        //Left border
        SDL_RenderDrawLine( renderer, x + i - 2, y + abs(i), x + i - 2, y + BOARD_HEIGHT + i );
        //Right border
        SDL_RenderDrawLine( renderer, x + BOARD_WIDTH + i + 1, y + abs(i), x + BOARD_WIDTH + i + 1, y + BOARD_HEIGHT - i );
        //Bottom border
        SDL_RenderDrawLine( renderer, x + i, y + BOARD_HEIGHT + i + 1, x + BOARD_WIDTH - i, y + BOARD_HEIGHT + i + 1);
    }
    displayBoardCell();
}


void Player::displayBoardCell()
{
    const float DURATION = 3000;
    const int FLASH_IO_DURATION = lockDelay / 5;

    //Scales the size of the cells.
    float scale = 1;

    //Repositions cells render origin to center it while changing their size. 
    int adjust = 0;
    bool needClearing = false;
    if ( gameOver )
    {
        scale = 1 + (SDL_GetTicks() - pullMark) * 2 / DURATION;
        tileSpriteSheet.setAlphaMod( max(255 - (int)((SDL_GetTicks() - pullMark) / DURATION * 255), 0) );
        adjust = - TILE_WIDTH * (scale - 1) / 2;
    }
    for ( int col = 0; col < WIDTH_BY_TILE ; col++ )
    {
        for ( int row = 0; row < HEIGHT_BY_TILE; row++ )
        {
            int cellState = pb.getCellState( row, col );
            if ( cellState > 0)
            {
                if ( cellState == CLEAR )
                {
                    tileSpriteSheet.setAlphaMod( 255 * max((FLASH_IO_DURATION - abs((int)(SDL_GetTicks() - lockMark - FLASH_IO_DURATION))), 0) / FLASH_IO_DURATION );
                    needClearing = true;
                }
                tileSpriteSheet.render( x + TILE_WIDTH * col + adjust, y + BOARD_HEIGHT - TILE_WIDTH * ( row + 1 ) + adjust, TILE_WIDTH * scale, TILE_WIDTH * scale, &tileSpriteClips[ cellState ] );
                if ( cellState == CLEAR ) tileSpriteSheet.setAlphaMod( 255 );
            }
        }
    }
    if ( needClearing )
    {
        if ( SDL_GetTicks() - lockMark < FLASH_IO_DURATION * 2 ) delaySpawnMark = SDL_GetTicks();
        else
        {
            pb.deleteClearedCell();
        }
    }
    if ( gameOver ) tileSpriteSheet.setAlphaMod( 255 );
}

void Player::displayCurrentTetromino()
{
    if ( tetr.getType() )
    {
        //Playfield's bottom left corner's position
        const int BOTTOM_LEFT_X = x;
        const int BOTTOM_LEFT_Y = y + BOARD_HEIGHT;

        for ( int row = 0; row < tetr.getContainerSize(); row++ )
        {
            for ( int col = 0; col < tetr.getContainerSize(); col++ )
            {
                if ( tetr.getCellState( row, col ) > 0 )
                {
                    int tile_x = BOTTOM_LEFT_X + ( tetr.getCol() + col ) * TILE_WIDTH;
                    int tile_y = BOTTOM_LEFT_Y - ( tetr.getRow() + row + 1) * TILE_WIDTH;
                    tileSpriteSheet.render( tile_x, tile_y, 
                                            TILE_WIDTH, TILE_WIDTH,
                                            &tileSpriteClips[ tetr.getCellState( row, col ) ]);
                    
                    //Renders this tile's ghost.
                    if (showGhost)
                    {
                        int ghostOffsetY = ( tetr.getRow() - getGhostRow() ) * TILE_WIDTH;
                        tileSpriteSheet.render( tile_x, tile_y + ghostOffsetY, 
                                                TILE_WIDTH, TILE_WIDTH, &tileSpriteClips[ 0 ]);
                    }
                }
            }
        }
    }
}

void Player::displayPreviewTetromino( int _x, int _y, const Tetromino &Ptetr )
{
    //Dimensions of a preview box.
    const int BOX_WIDTH = TILE_WIDTH * 3;
    const int BOX_HEIGHT = TILE_WIDTH * 2;
    const int PREVIEW_TILE_WIDTH = TILE_WIDTH * 2 / 3;
    //Adjusts Y to center the tetromino in preview box, offsetX is always half the container's dimension. 
    int offsetX, offsetY;

    //Center point of the preview box.
    const int CENTER_X = BOX_WIDTH / 2 + _x;
    const int CENTER_Y = BOX_HEIGHT / 2 + _y;

    if ( Ptetr.getType() == I_PIECE )           offsetY = PREVIEW_TILE_WIDTH * 5 / 2;
    else if ( Ptetr.getType() == BOMB_PIECE)    offsetY = PREVIEW_TILE_WIDTH * 3 / 2;
    else                                        offsetY = PREVIEW_TILE_WIDTH * 2;

    offsetX = Ptetr.getContainerSize() * PREVIEW_TILE_WIDTH / 2 / (Ptetr.getContainerSize() > 4 ? 2 : 1);

    for ( int row = 0; row < Ptetr.getContainerSize(); row++ )
        for ( int col = 0; col < Ptetr.getContainerSize(); col++ )
            if ( Ptetr.getCellState( row, col ) != 0 )
                tileSpriteSheet.render( CENTER_X - offsetX + col * PREVIEW_TILE_WIDTH / (Ptetr.getContainerSize() > 4 ? 2 : 1),
                                        CENTER_Y + offsetY - (row + 1) * PREVIEW_TILE_WIDTH / (Ptetr.getContainerSize() > 4 ? 2 : 1),
                                        PREVIEW_TILE_WIDTH / (Ptetr.getContainerSize() > 4 ? 2 : 1), PREVIEW_TILE_WIDTH / (Ptetr.getContainerSize() > 4 ? 2 : 1),
                                        &tileSpriteClips[ Ptetr.getCellState( row, col ) ] );
}

void Player::displayTetrominoQueue( vector<Tetromino> &Tqueue, int queuePosition )
{
    //padding between queue and playfield
    const int PADDING = TILE_WIDTH * 2 / 3;

    //Position of the queue container's top left corner
    const int TOP_LEFT_X = x + BOARD_WIDTH + PADDING;
    const int TOP_LEFT_Y = y;
    
    const int BOX_HEIGHT = TILE_WIDTH * 2;
    
    for ( int i = 0; i < min(nextBoxes, (int)Tqueue.size()); i++ )
    {
        displayPreviewTetromino( TOP_LEFT_X, TOP_LEFT_Y + i * BOX_HEIGHT, Tqueue[ queuePosition + i ] );
    }
}

void Player::displayHeldTetromino()
{
    if ( hold.getType() )
    {
        displayPreviewTetromino( x - TILE_WIDTH * 4, y, hold );
    }
}

void Player::ingameProgress( const vector<Tetromino> &Tqueue, int &queuePosition, int &scene )
{
    //Initialize game & piece bag;
    const int SPAWN_DELAY = 16;
    if ( !gameOver )
    {   
        if ( tetr.getType() == 0 )
        {
            if ( (int)SDL_GetTicks() - (int)delaySpawnMark > SPAWN_DELAY )
            {
                pullNewTetromino( Tqueue, queuePosition );
                if ( tetr.getType() != BOMB_PIECE && checkCollision( tetr ) ) terminateGame();
            }
        }
        if ( tetr.getType() != 0 ) gravityPull();

        if ( !gameOver && activePlayer ) handlingKeyPress( gameOver, scene );
    }
}

void Player::handlingKeyPress( bool &gameOver, int &scene )
{
    //Movement keys are allowed to be held for auto moving the pieces. When a key is held, it sends one input first, then auto repeat after a short delay
    //DAS: Delay auto-shift
    //ARR: Auto-repeat rate (Hz)
    const int DAS = min(1000/6, lockDelay / 2), ARR = 30;
    const int REPEAT_INTERVAL = 1000 / ARR;

    //Handle events
    SDL_Event event;
    while ( SDL_PollEvent(&event) > 0 ) 
    {
        //Quits game event
        if (  event.type == SDL_QUIT )
        {
            gameOver = true;
            scene = QUIT;
            break;
        }
    }

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    //KEY STATES:
    //0: Unpressed
    //1: Pressed once, waiting for DAS (or KEYUP event if the key is non-repeat)
    //2: Auto-repeat
    //3: Hold for handling input of moving in the opposite direction.
    if ( (keystate[keyScanCode[PRIMARY_HARD_DROP]] || keystate[keyScanCode[SECONDARY_HARD_DROP]]) )
    {
        if ( keyRepeatState[K_SPACE].first == 0 )
        {
            if ( mode == MYSTERY && tetr.getType() == BOMB_PIECE )
            {
                lockTetromino();
            }
            else dropPiece( true );
            keyRepeatState[K_SPACE].first = 1;
        }
    } else keyRepeatState[K_SPACE].first = 0;

    if ( (keystate[keyScanCode[PRIMARY_RIGHT_ROTATE]] || keystate[keyScanCode[SECONDARY_RIGHT_ROTATE]]) )
    {
        if (keyRepeatState[K_RROTATE].first == 0)
        {
            rotatePiece( true );
            keyRepeatState[K_RROTATE].first = 1;
        }
    } else keyRepeatState[K_RROTATE].first = 0;

    if ( (keystate[keyScanCode[PRIMARY_LEFT_ROTATE]] || keystate[keyScanCode[SECONDARY_LEFT_ROTATE]]) )
    {
        if ( keyRepeatState[K_LROTATE].first == 0 )
        {
            rotatePiece( false );
            keyRepeatState[K_LROTATE].first = 1;
        }
    } else keyRepeatState[K_LROTATE].first = 0;

    if ( (keystate[keyScanCode[PRIMARY_SWAP_HOLD]] || keystate[keyScanCode[SECONDARY_SWAP_HOLD]]) )
    {
        if ( keyRepeatState[K_SWAP].first == 0 )
        {
            swapHoldPiece();
            keyRepeatState[K_SWAP].first = 1;
        }
    } else keyRepeatState[K_SWAP].first = 0;

    if ( keystate[SDL_SCANCODE_ESCAPE] )
    {
        if ( mode <= MYSTERY && keyRepeatState[K_ESC].first == 0 )
        {
            scene = PAUSE;
            keyRepeatState[K_ESC].first = 1;
        }
    } else keyRepeatState[K_ESC].first = 0;

    if ( keystate[keyScanCode[PRIMARY_MOVE_LEFT]] || keystate[keyScanCode[SECONDARY_MOVE_LEFT]] ) 
    {
        if ( keyRepeatState[K_LEFT].first == 0 )
        {
            if (tetr.getType() != 0) movePieceHorizontally( false );
            keyRepeatState[K_LEFT].first = 1;
            keyRepeatState[K_LEFT].second = SDL_GetTicks();
        }
        else if ( keyRepeatState[K_LEFT].first == 1 && SDL_GetTicks() - keyRepeatState[K_LEFT].second > DAS ) { keyRepeatState[K_LEFT].first = 2; }
        if ( keyRepeatState[K_LEFT].first == 2 && SDL_GetTicks() - keyRepeatState[K_LEFT].second > ARR)
        {
            if ( keyRepeatState[K_RIGHT].first == 1 ) keyRepeatState[K_LEFT].first = 3;
            else
            {
                if (tetr.getType() != 0) movePieceHorizontally( false );
                keyRepeatState[K_LEFT].second = SDL_GetTicks();
            }
        }
        else if ( keyRepeatState[K_LEFT].first == 3 && keyRepeatState[K_RIGHT].first == 0 ) keyRepeatState[K_LEFT].first = 2;
    } else keyRepeatState[K_LEFT].first = 0;

    if ( keystate[keyScanCode[PRIMARY_MOVE_RIGHT]] || keystate[keyScanCode[SECONDARY_MOVE_RIGHT]] ) 
    {
        if ( keyRepeatState[K_RIGHT].first == 0 )
        {
            if (tetr.getType() != 0) movePieceHorizontally( true );
            keyRepeatState[K_RIGHT].first = 1;
            keyRepeatState[K_RIGHT].second = SDL_GetTicks();
        }
        else if ( keyRepeatState[K_RIGHT].first == 1 && SDL_GetTicks() - keyRepeatState[K_RIGHT].second > DAS ) { keyRepeatState[K_RIGHT].first = 2; }
        if ( keyRepeatState[K_RIGHT].first == 2 && SDL_GetTicks() - keyRepeatState[K_RIGHT].second > ARR)
        {
            if ( keyRepeatState[K_LEFT].first == 1 ) keyRepeatState[K_RIGHT].first = 3;
            else
            {
                if (tetr.getType() != 0) movePieceHorizontally( true );
                keyRepeatState[K_RIGHT].second = SDL_GetTicks();
            }
        }
        else if ( keyRepeatState[K_RIGHT].first == 3 && keyRepeatState[K_LEFT].first == 0 ) keyRepeatState[K_RIGHT].first = 2;
    } else keyRepeatState[K_RIGHT].first = 0;

    
    if ( keystate[keyScanCode[PRIMARY_SOFT_DROP]] || keystate[keyScanCode[SECONDARY_SOFT_DROP]] )
    {
        if ( keyRepeatState[K_DOWN].first == 0 || (keyRepeatState[K_DOWN].first == 2 && SDL_GetTicks() - keyRepeatState[K_DOWN].second > ARR))
        {
            if (tetr.getType() != 0) dropPiece( false );
            keyRepeatState[K_DOWN].second = SDL_GetTicks();
            if ( keyRepeatState[K_DOWN].first == 0 ) keyRepeatState[K_DOWN].first = 2;
        }
    } else keyRepeatState[K_DOWN].first = 0;
    if ( keystate[SDL_SCANCODE_X] || keystate[SDL_SCANCODE_UP] && tetr.getType() == 0 )
    {
        irsCharge = 1;
    }
    else if ( keystate[SDL_SCANCODE_Z] && tetr.getType() == 0 )
    {
        irsCharge = -1;
    } else irsCharge = 0;
}