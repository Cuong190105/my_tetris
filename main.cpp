#include <iostream>
#include <cstdlib>
#include <vector>
#include "logic.hpp"
using namespace std;

//--------------------------INIT & LOAD--------------------------

//Initializes SDL system, game window and renderer
bool init();

//Loads media
void loadMedia();

//Loads random background
void loadRandomBackground();

//Terminate SDL system and close window
void close();

int main(int argv, char **args) {
    //Initialize SDL & Create window

    if ( !init() ) {
        cout << "Failed to initialize" << endl;
    }
    else
    {
        srand(time(NULL));
        loadMedia();
        taskManager();
    }
    close();
    saveSettings();
    return 0;
}

bool init()
{
    //Initialization status flag
    bool success = true;

    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) < 0 )
    {
        cout << "Failed to initialize SDL" << endl;
        success = false;
    }
    else
    {
        SDL_DisplayMode dm;
        SDL_GetDisplayMode(0, 0, &dm);
        maxHeight = dm.h;
        loadSettingsFromFile();
        applySettings();
        //Set hint
        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

        //Create game window
        game_window = SDL_CreateWindow( "Homemade Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
        if ( game_window == NULL )
        {
            cout << "Failed to create window" << endl;
            success = false;
        }
        else 
        {
            //Create renderer
            renderer = SDL_CreateRenderer( game_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC );
            if ( renderer == NULL )
            {
                cout << "Failed to create renderer" << endl;
                success = false;
            }
            else
            {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
                int audioFlags = MIX_INIT_MP3;
                if ( !( Mix_Init( audioFlags ) && audioFlags ) )
                {
                    cout << "SDL_Mixer could not initialize! SDL_mixer Error:" << Mix_GetError() << endl;
                    success = false;
                }
                else
                {
                    if( Mix_OpenAudio( 48000, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                    {
                        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                        success = false;
                    }
                }
            }
        }
    }
    return success;
}

void loadMedia()
{
    const string TILE_SPRITE_SHEET_PATH = "src/media/img/Tile_sheet.png";
    const string AUDIO_PATH = "/Tile_sheets.png";

    //Loads sprite sheet
    if ( !tileSpriteSheet.loadFromFile( TILE_SPRITE_SHEET_PATH ) )
    {
        cout << "Failed to load Tile sprite sheet." << endl;
    }
    else
    {
        for ( int i = 0; i < 4; i++ )
        {    
            tileSpriteClips[ i ].x = 300 * i;
		    tileSpriteClips[ i ].y = 0;
		    tileSpriteClips[ i ].w = 300;
		    tileSpriteClips[ i ].h = 300;

            tileSpriteClips[ i + 4 ].x = 300 * i;
		    tileSpriteClips[ i + 4 ].y = 300;
		    tileSpriteClips[ i + 4 ].w = 300;
		    tileSpriteClips[ i + 4 ].h = 300;

            tileSpriteClips[ i + 8 ].x = 300 * i;
		    tileSpriteClips[ i + 8 ].y = 600;
		    tileSpriteClips[ i + 8 ].w = 300;
		    tileSpriteClips[ i + 8 ].h = 300;
        }
    }


    //Load font
    fontBold = TTF_OpenFont("src/media/fonts/gameFontBold.ttf", LENGTH_UNIT);
    if ( fontBold == NULL )
    {
        cout << "Failed to load font." << endl;
    }
    fontRegular = TTF_OpenFont("src/media/fonts/gameFontRegular.ttf", LENGTH_UNIT);
    if ( fontRegular == NULL )
    {
        cout << "Failed to load font." << endl;
    }
    //Load all Sfx
    loadSfx();
}

void close()
{
    //Destroy all textures
    tileSpriteSheet.free();
    textTexture.free();
    bgImage.free();
    menuBackground.free();
    //Free font
    TTF_CloseFont( fontBold );
    TTF_CloseFont( fontRegular );
    fontBold = NULL;
    fontRegular = NULL;
    //Free sounds
    for (int i = 0; i < SFX; i++) { Mix_FreeChunk( sfx[i] ); sfx[i] = NULL; }
    Mix_FreeMusic( background_music );
    background_music = NULL;
    //Destroy window
    SDL_DestroyRenderer( renderer );
    renderer = NULL;

    SDL_DestroyWindow( game_window );
    game_window = NULL;

    //Quit SDL
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}