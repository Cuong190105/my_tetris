#ifndef Texture_hpp
#define Texture_hpp
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
using namespace std;

const string SLIDER_BG = "src/media/img/bg_slider.png";
const string SLIDER_FG = "src/media/img/fg_slider.png";
const string SLIDER_HEAD = "src/media/img/slider_head.png";
const string GAME_TITLE = "src/media/img/game_title.png";
const string MENU_BACKGROUND_PATH = "src/media/img/menu_bg.png";
const string TILE_SPRITE_SHEET_PATH = "src/media/img/Tile_sheet.png";
const string AUDIO_PATH = "/Tile_sheets.png";
const char* const FONT_BOLD_PATH = "src/media/fonts/gameFontBold.ttf";
const char* const FONT_REGULAR_PATH = "src/media/fonts/gameFontRegular.ttf";

extern SDL_Renderer *renderer;
extern TTF_Font *fontBold;
extern TTF_Font *fontRegular;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

class Texture
{
    private:
        SDL_Texture *texture;
        int width, height;
    public:
        Texture();
        ~Texture();

        //Manual Deallocator
        void free();

        //Loads text
        bool loadText( string text, TTF_Font *font, SDL_Color color = {255, 255, 255});

        //Loads texture from file
        bool loadFromFile( string path );

        void setColorMod( Uint8 r, Uint8 g, Uint8 b );

        void setAlphaMod( Uint8 a );

        /**
         * Renders texture at a given position
         * \param x, y coordinate of destination frame's top left corner
         * \param w width of the destination frame
         * \param h height of the destination frame
         * \param clip a pointer to a SDL_Rect indicating part of the texture being copied, leave NULL to copy the whole texture 
         */
        void render( int x = 0, int y = 0, int w = WINDOW_WIDTH, int h = WINDOW_HEIGHT, SDL_Rect* clip = NULL, int angle = 0 );

        //Returns width
        int getWidth();

        //Returns height
        int getHeight();
};

enum gameScene {MAIN_MENU, SOLO_MENU, MULTI_MENU, SETTINGS, QUIT, SET_RULES, INGAME, PAUSE};
extern SDL_Rect tileSpriteClips[12];
extern Texture tileSpriteSheet;
extern Texture textTexture;
extern Texture bgImage;
extern Texture menuBackground;
#endif