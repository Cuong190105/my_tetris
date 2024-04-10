#ifndef settings_hpp
#define settings_hpp
#include <fstream>
#include <SDL.h>
#include <string>
using namespace std;
enum keyFunc
{
    PRIMARY_LEFT_ROTATE,
    SECONDARY_LEFT_ROTATE,
    PRIMARY_RIGHT_ROTATE,
    SECONDARY_RIGHT_ROTATE,
    PRIMARY_SWAP_HOLD,
    SECONDARY_SWAP_HOLD,
    PRIMARY_MOVE_LEFT,
    SECONDARY_MOVE_LEFT,
    PRIMARY_MOVE_RIGHT,
    SECONDARY_MOVE_RIGHT,
    PRIMARY_SOFT_DROP,
    SECONDARY_SOFT_DROP,
    PRIMARY_HARD_DROP,
    SECONDARY_HARD_DROP,
    NUM_KEY_FUNCTIONS,
};

enum cfg { RESOLUTION, BGM_VOLUME, SFX_VOLUME, SHOW_GHOST, NEXT_BOXES, PLAYFIELD_SCALE, NUM_CFG };
extern const int LENGTH_UNIT;
extern const string CONFIG_FILE;
extern int heightDimension;
extern int bgmVolume;
extern int sfxVolume;
extern bool showGhost;
extern double playfieldScale;
extern int nextBoxes;
extern int keyScanCode[NUM_KEY_FUNCTIONS];

int extractNum( string line );

void loadSettingsFromFile();

void saveSettings();
#endif