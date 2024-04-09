#include "audio.hpp"

#include <string>
using namespace std;

Mix_Music *background_music;
Mix_Chunk *sfx[SFX];

void playBackgroundMusic(int track)
{
    const string bg_sound[] = {
        "background_menu_music.mp3",
        "tetris_theme_chill.mp3",
        "tetris_theme_fast.MP3",
    };
    background_music = Mix_LoadMUS(("src/media/audio/" + bg_sound[track]).c_str());
    Mix_PlayMusic( background_music, -1 );
}

void loadSfx()
{
    const string sfx_list[SFX] = {
        "bonus_point",
        "countdown",
        "gameover",
        "harddrop",
        "line_clear",
        "lock",
        "move",
        "select",
        "tetris_bonus",
        "victory",
    };
    for (int i = 0; i < SFX; i++) sfx[i] = Mix_LoadWAV(("src/media/audio/" + sfx_list[i] + ".wav").c_str());
}

void playSfx(int fx)
{
    Mix_PlayChannel(-1, sfx[fx], 0);
}

void stopMusic( bool fade )
{
    if (!fade) Mix_HaltMusic();
    else Mix_FadeOutMusic(500);
}

void pauseMusic()
{
    Mix_PauseMusic();
}

void resumeMusic()
{
    Mix_ResumeMusic();
}