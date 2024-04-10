#ifndef audio_hpp
#define audio_hpp
#include <SDL_mixer.h>
#include "settings.hpp"
extern Mix_Music *background_music;

enum BgTrack {MENU, CHILL_THEME, FAST_THEME};
void playBackgroundMusic(int track);

const int SFX = 10;
extern Mix_Chunk *sfx[];
enum SoundEffect {BONUS_POINT, COUNTDOWN, GAMEOVER, HARDDROP, LINE_CLEAR, LOCK, MOVE, SELECT, TETRIS_BONUS, VICTORY};
void loadSfx();
void playSfx(int fx);
void stopMusic( bool fade );
void resumeMusic();
void changeSfxVolume();
void changeBgmVolume();
#endif