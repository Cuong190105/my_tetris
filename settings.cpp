#include "settings.hpp"
#include <iostream>
#include <iomanip>
using namespace std;
const string CONFIG_FILE = "settings.txt";
int heightDimension = 0;
int LENGTH_UNIT = heightDimension / 36;
int bgmVolume = 100;
int sfxVolume = 100;
bool showGhost = 1;
double playfieldScale = 1;
int nextBoxes = 5;
vector<vector<vector<int>>> hiscore ( 5, vector<vector<int>> (5, vector<int> (3, 0 ) ));
SDL_Scancode keyScanCode[NUM_KEY_FUNCTIONS] =
{
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_UNKNOWN,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_UNKNOWN,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_UNKNOWN,
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_UNKNOWN,
    SDL_SCANCODE_C,
    SDL_SCANCODE_UNKNOWN,
    SDL_SCANCODE_X,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_UNKNOWN,
};
int maxHeight;
const int HEIGHT_ALLOWED[] = { 720, 768, 900, 1080, 1440, 2160 };

const string CFG_NAME[] =
{
    "resolution",
    "bgm_volume",
    "sfx_volume",
    "show_ghost",
    "next_boxes",
    "playfield_element_scale",
    "primary_move_left",
    "secondary_move_left",
    "primary_move_right",
    "secondary_move_right",
    "primary_soft_drop",
    "secondary_soft_drop",
    "primary_hard_drop",
    "secondary_hard_drop",
    "primary_swap_hold",
    "secondary_swap_hold",
    "primary_right_rotate",
    "secondary_right_rotate",
    "primary_left_rotate",
    "secondary_left_rotate",
};

double extractFloat( string line )
{
    double data = 0;
    int tens = 0;
    bool equalSignPassed = false;
    bool decimalpointPassed = false;
    for ( int i = 0; i < line.length(); i++ )
    {
        if (line[i] == '=') equalSignPassed = true;
        else if (equalSignPassed)
        {
            if (line[i] == '.' ) decimalpointPassed = true;
            else
            {
                data = data * 10 + (line[i] - '0');
                if (decimalpointPassed) tens ++;
            }
        }
    }
    for ( int i = 0; i < tens; i++ ) data /= 10;
    return data;
}

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

void validateConfig()
{
    for ( int i = 0; i < 6 && HEIGHT_ALLOWED[i] <= maxHeight; i++ )
    {
        if ( heightDimension == HEIGHT_ALLOWED[i] ) break;
        else if ( heightDimension < HEIGHT_ALLOWED[i] ) {heightDimension = HEIGHT_ALLOWED[max(i - 1, 0)]; break; }
        else if ( i == 5 || HEIGHT_ALLOWED[i + 1] > maxHeight ) heightDimension = HEIGHT_ALLOWED[i];
    }
    if ( bgmVolume < 0) bgmVolume = 0;
    else if ( bgmVolume > 100) bgmVolume = 100;

    if ( sfxVolume < 0) sfxVolume = 0;
    else if ( sfxVolume > 100) sfxVolume = 100;

    if ( playfieldScale < 0.5) playfieldScale = 0.5;
    else if ( playfieldScale > 1.34) playfieldScale = 1.34;
    if ( nextBoxes < 1) nextBoxes = 1;
    else if ( nextBoxes > 5) nextBoxes = 5;
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
        playfieldScale = extractFloat( line );

        validateConfig();

        for ( int i = 0; i < NUM_KEY_FUNCTIONS; i++ )
        {
            settings >> line;
            keyScanCode[i] = SDL_Scancode(extractNum( line ));
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
    if ( heightDimension == 0 )
    {
        for ( int i = 5; i > 0; i-- )
        {
            if ( maxHeight > HEIGHT_ALLOWED[i] )
            {
                heightDimension = HEIGHT_ALLOWED[i];
                break;
            }
        }
        if ( heightDimension == 0 ) heightDimension = HEIGHT_ALLOWED[0];

    }
    settings << CFG_NAME[RESOLUTION] << '=' << heightDimension << endl;
    settings << CFG_NAME[BGM_VOLUME] << '=' << bgmVolume << endl;
    settings << CFG_NAME[SFX_VOLUME] << '=' << sfxVolume << endl;
    settings << CFG_NAME[SHOW_GHOST] << '=' << showGhost << endl;
    settings << CFG_NAME[NEXT_BOXES] << '=' << nextBoxes << endl;
    settings << CFG_NAME[PLAYFIELD_SCALE] << '=' << fixed << setprecision(4) << playfieldScale << endl;
    for (int i = 0; i < NUM_KEY_FUNCTIONS; i++)
    {
        settings << CFG_NAME[NUM_CFG + i] << '=' << (int)keyScanCode[i] << endl;
    }
    settings.close();
}