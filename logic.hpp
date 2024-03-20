#ifndef logic_hpp
#define logic_hpp
#include "Player.hpp"
#include <SDL.h>
using namespace std;

void generateTetromino( vector<int> &Tqueue );

void pullNewTetromino( vector<Tetromino> &Tqueue, Tetromino &tetr );

void autoLockHandler( Player &player, bool reset = false, const Uint32 &lockMark = 0, bool moved = false);

void handlingKeyPress( SDL_Event &e, Player &player, Uint32 &pullMark, bool &activeLockTimer );

int pullInterval( int level );


/**
 * Pulls the tetromino down (simulating gravity) after an interval. Pull timer resets when
 * the player manually drops the piece. When the piece is on top of the stack (can no longer fall down),
 * this function triggers a lock timer that will lock the piece automatically after the timer expires.
 * \param player Player's field we want to pull
 * \param pullMark Marks the last time pull timer reset
 * \param activeLockTimer Get value true if the tetromino is on top of the stack
*/
void gravityPull( Player& player, Uint32 &pullMark, bool activeLockTimer, Uint32 &lockMark, bool &moved );

void ingameProgress();
#endif