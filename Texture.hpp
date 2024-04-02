#ifndef Texture_hpp
#define Texture_hpp
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
using namespace std;

extern SDL_Renderer *renderer;
extern TTF_Font *fontBold;
extern TTF_Font *fontRegular;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

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
extern SDL_Rect tileSpriteClips[8];
extern Texture tileSpriteSheet;
extern Texture textTexture;
extern Texture bgImage;
extern Texture menuBackground;
#endif