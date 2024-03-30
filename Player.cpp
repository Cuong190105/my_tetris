#include "Player.hpp"

#include <cmath>
#include <iostream>
using namespace std;

enum Action {ROTATE, MOVE_LR, DROP};

Player::Player( int _level, int _mode, int _x, int _y )
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
    lockDelay = 500;
    mode = _mode;
    if (mode == MASTER) pullInterval = 1;
    setLevel(_level);
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

void Player::terminateGame() { gameOver = true; }
bool Player::isGameOver() { return gameOver; }


void Player::setX( int _x ) { x = _x; }
int Player::getX() const{ return x; }

void Player::setY( int _y ) { y = _y; }
int Player::getY() const { return y; }

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
    if ( movesBeforeLock >= 15 || SDL_GetTicks() - lockMark > lockDelay ) dropPiece( true );
}

void Player::gravityPull()
{
    if ( checkCollision( tetr, -1 ) )
    {
        lockDelayHandler();
    }
    else
    {
        int rowDrop = 10 / pullInterval + 1;
        int newPullInterval = pullInterval * rowDrop;
        if ( ( SDL_GetTicks() - pullMark ) > newPullInterval )
        {
            for ( int i = 0; i < rowDrop; i++)
            {
                dropPiece( false, true );
            }
            pullMark = SDL_GetTicks();
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

void Player::displayBoard()
{
    //Draw board background color
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0xFF );
    SDL_Rect board { x, y, BOARD_WIDTH, BOARD_HEIGHT };
    SDL_RenderFillRect( renderer, &board );

    //Draw board gridlines
    SDL_SetRenderDrawColor( renderer, 0x22, 0x22, 0x22, 0xFF );
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

    // Draw pieces on the board;
    for ( int row = 0; row < HEIGHT_BY_TILE; row++ )
    {
        for ( int col = 0; col < WIDTH_BY_TILE; col++ )
        {
            int cellState = pb.getCellState( row, col );
            if ( cellState > 0)
            {
                tileSpriteSheet.render( x + TILE_WIDTH * col, y + BOARD_HEIGHT - TILE_WIDTH * ( row + 1 ), TILE_WIDTH, TILE_WIDTH, &tileSpriteClips[ cellState ] );
            }
        }
    }
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
                    int ghostOffsetY = ( tetr.getRow() - getGhostRow() ) * TILE_WIDTH;
                    tileSpriteSheet.render( tile_x, tile_y + ghostOffsetY, 
                                            TILE_WIDTH, TILE_WIDTH, &tileSpriteClips[ 0 ]);
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

    if ( Ptetr.getType() == I_PIECE )    offsetY = PREVIEW_TILE_WIDTH * 5 / 2;
    else                                offsetY = PREVIEW_TILE_WIDTH * 2;

    offsetX = Ptetr.getContainerSize() * PREVIEW_TILE_WIDTH / 2;

    for ( int row = 0; row < Ptetr.getContainerSize(); row++ )
        for ( int col = 0; col < Ptetr.getContainerSize(); col++ )
            if ( Ptetr.getCellState( row, col ) != 0 )
                tileSpriteSheet.render( CENTER_X - offsetX + col * PREVIEW_TILE_WIDTH,
                                        CENTER_Y + offsetY - (row + 1) * PREVIEW_TILE_WIDTH,
                                        PREVIEW_TILE_WIDTH, PREVIEW_TILE_WIDTH,
                                        &tileSpriteClips[ Ptetr.getCellState( row, col ) ] );
}

void Player::displayTetrominoQueue( vector<Tetromino> &Tqueue, int previewPieces, int queuePosition )
{
    //padding between queue and playfield
    const int PADDING = TILE_WIDTH * 2 / 3;

    //Position of the queue container's top left corner
    const int TOP_LEFT_X = x + BOARD_WIDTH + PADDING;
    const int TOP_LEFT_Y = y;
    
    const int BOX_HEIGHT = TILE_WIDTH * 2;
    
    for ( int i = 0; i < min(previewPieces, (int)Tqueue.size()); i++ )
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
    if ( !gameOver )
    {   
        if ( tetr.getType() == 0 )
        {
            pullNewTetromino( Tqueue );
            queuePosition++;
        }
        if ( checkCollision( tetr ) ) gameOver = true;
        gravityPull();

        if ( !gameOver ) handlingKeyPress( gameOver, scene );
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
    while ( SDL_PollEvent(&event) != 0 ) 
    {
        //Quits game event
        if (  event.type == SDL_QUIT )
        {
            gameOver = true;
            scene = QUIT;
            break;
        }
        //Handles non-repeat keys
        if ( event.type == SDL_KEYDOWN && event.key.repeat == 0 )
        {
            switch( event.key.keysym.sym )
            {
                case SDLK_SPACE:
                    dropPiece( true );
                    break;
                //Rotates clockwise
                case SDLK_UP:
                case SDLK_x:
                //Rotates counterclockwise
                case SDLK_z:
                    rotatePiece( event.key.keysym.sym != SDLK_z );
                    break;
                case SDLK_c:
                    swapHoldPiece();
                    break;
            }
        }
    }

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    //KEY STATES:
    //0: Unpressed
    //1: Pressed once, waiting for DAS
    //2: Auto-repeat
    //3: Hold for handling input of moving in the opposite direction.
    if ( keystate[SDL_SCANCODE_LEFT] ) 
    {
        if ( keyRepeatState[K_LEFT].first == 0 )
        {
            movePieceHorizontally( false );
            keyRepeatState[K_LEFT].first = 1;
            keyRepeatState[K_LEFT].second = SDL_GetTicks();
        }
        else if ( keyRepeatState[K_LEFT].first == 1 && SDL_GetTicks() - keyRepeatState[K_LEFT].second > DAS ) { keyRepeatState[K_LEFT].first = 2; }
        if ( keyRepeatState[K_LEFT].first == 2 && SDL_GetTicks() - keyRepeatState[K_LEFT].second > ARR)
        {
            if ( keyRepeatState[K_RIGHT].first == 1 ) keyRepeatState[K_LEFT].first = 3;
            else
            {
                movePieceHorizontally( false );
                keyRepeatState[K_LEFT].second = SDL_GetTicks();
            }
        }
        else if ( keyRepeatState[K_LEFT].first == 3 && keyRepeatState[K_RIGHT].first == 0 ) keyRepeatState[K_LEFT].first = 2;
    } else keyRepeatState[K_LEFT].first = 0;

    if ( keystate[SDL_SCANCODE_RIGHT] ) 
    {
        if ( keyRepeatState[K_RIGHT].first == 0 )
        {
            movePieceHorizontally( true );
            keyRepeatState[K_RIGHT].first = 1;
            keyRepeatState[K_RIGHT].second = SDL_GetTicks();
        }
        else if ( keyRepeatState[K_RIGHT].first == 1 && SDL_GetTicks() - keyRepeatState[K_RIGHT].second > DAS ) { keyRepeatState[K_RIGHT].first = 2; }
        if ( keyRepeatState[K_RIGHT].first == 2 && SDL_GetTicks() - keyRepeatState[K_RIGHT].second > ARR)
        {
            if ( keyRepeatState[K_LEFT].first == 1 ) keyRepeatState[K_RIGHT].first = 3;
            else
            {
                movePieceHorizontally( true );
                keyRepeatState[K_RIGHT].second = SDL_GetTicks();
            }
        }
        else if ( keyRepeatState[K_RIGHT].first == 3 && keyRepeatState[K_LEFT].first == 0 ) keyRepeatState[K_RIGHT].first = 2;
    } else keyRepeatState[K_RIGHT].first = 0;

    
    if ( keystate[SDL_SCANCODE_DOWN] )
    {
        if ( keyRepeatState[K_DOWN].first == 0 || (keyRepeatState[K_DOWN].first == 2 && SDL_GetTicks() - keyRepeatState[K_DOWN].second > ARR))
        {
            dropPiece( false );
            keyRepeatState[K_DOWN].second = SDL_GetTicks();
            if ( keyRepeatState[K_DOWN].first == 0 ) keyRepeatState[K_DOWN].first = 2;
        }
    } else keyRepeatState[K_DOWN].first = 0;
    
    // if ( e.type == SDL_KEYDOWN )
    // {
    //     // cout << SDL_GetKeyName(e.key.keysym.sym) << endl;
    //         //Drops
    //         case SDLK_DOWN:
    //         //Moves K_left or right
    //         case SDLK_K_LEFT:
    //         case SDLK_RIGHT:
                // movePieceHorizontally( e.key.keysym.sym == SDLK_RIGHT );
    //             break;
    // }
}