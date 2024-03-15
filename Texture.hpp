#ifndef Texture_hpp
#define Texture_hpp
#include <SDL.h>
#include <string>
using namespace std;

extern SDL_Renderer *renderer;

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

        //Loads texture from file
        bool loadFromFile( string path );

        /**
         * Renders texture at a given position
         * \param x, y coordinate of destination frame's top left corner
         * \param w width of the destination frame
         * \param h height of the destination frame
         * \param clip a pointer to a SDL_Rect indicating part of the texture being copied, leave NULL to copy the whole texture 
         */
        void render( int x, int y, int w, int h, SDL_Rect* clip = NULL );

        //Returns width
        int getWidth();

        //Returns height
        int getHeight();
};

extern SDL_Rect tileSpriteClips[8];
extern Texture tileSpriteSheet;

#endif