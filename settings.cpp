#include "settings.hpp"
#include <iostream>
using namespace std;
const int LENGTH_UNIT = 30;
const string CONFIG_FILE = "settings.txt";
int heightDimension = 720;
int bgmVolume = 100;
int sfxVolume = 100;
bool showGhost = 1;
double playfieldScale = 1;
int nextBoxes = 5;
int keyScanCode[NUM_KEY_FUNCTIONS] = { 29, 0, 27, 82, 6, 0, 80, 0, 79, 0, 81, 0, 44, 0 };

const string CFG_NAME[] =
{
    "resolution",
    "bgm_volume",
    "sfx_volume",
    "show_ghost",
    "next_boxes",
    "playfield_element_scale",
    "primary_left_rotate",
    "secondary_left_rotate",
    "primary_right_rotate",
    "secondary_right_rotate",
    "primary_swap_hold",
    "secondary_swap_hold",
    "primary_move_left",
    "secondary_move_left",
    "primary_move_right",
    "secondary_move_right",
    "primary_soft_drop",
    "secondary_soft_drop",
    "primary_hard_drop",
    "secondary_hard_drop",
};

int extractNum( string line )
{
    int data = 0;
    bool equalSignPassed = false;
    for ( int i = 0; i < line.length(); i++ )
    {
        if (line[i] == '=') equalSignPassed = true;
        else if (equalSignPassed) data = data * 10 + (line[i] - '0');
    }
    return data;
}

void loadSettingsFromFile()
{
    ifstream settings(CONFIG_FILE);
    if (settings.is_open())
    {
        string line;
        settings >> line;
        heightDimension = extractNum( line );

        settings >> line;
        bgmVolume = extractNum( line );

        settings >> line;
        sfxVolume = extractNum( line );
        
        settings >> line;
        showGhost = extractNum( line );
        
        settings >> line;
        nextBoxes = extractNum( line );

        settings >> line;
        playfieldScale = extractNum( line );

        for ( int i = 0; i < NUM_KEY_FUNCTIONS; i++ )
        {
            settings >> line;
            keyScanCode[i] = extractNum( line );
        }
        settings.close();
    }
    else
    {
        saveSettings();
    }
}

void saveSettings()
{
    ofstream settings(CONFIG_FILE);
    settings << CFG_NAME[RESOLUTION] << '=' << heightDimension << endl;
    settings << CFG_NAME[BGM_VOLUME] << '=' << bgmVolume << endl;
    settings << CFG_NAME[SFX_VOLUME] << '=' << sfxVolume << endl;
    settings << CFG_NAME[SHOW_GHOST] << '=' << showGhost << endl;
    settings << CFG_NAME[NEXT_BOXES] << '=' << nextBoxes << endl;
    settings << CFG_NAME[PLAYFIELD_SCALE] << '=' << playfieldScale << endl;
    for (int i = 0; i < NUM_KEY_FUNCTIONS; i++)
    {
        settings << CFG_NAME[NUM_CFG + i] << '=' << keyScanCode[i] << endl;
    }
    settings.close();
}