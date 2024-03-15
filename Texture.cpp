#include "Texture.hpp"
#include <SDL_image.h>
using namespace std;
SDL_Rect tileSpriteClips[8];

//Stores the tile sprite sheet
Texture tileSpriteSheet;

SDL_Renderer *renderer = NULL;
Texture::Texture()
{
    texture = NULL;
    width = 0;
    height = 0;
}

Texture::~Texture()
{
    free();
}

void Texture::free()
{
    if ( texture != NULL )
    {
        SDL_DestroyTexture( texture );
        texture = NULL;
        width = 0;
        height = 0;
    }
}

bool Texture::loadFromFile( string path )
{
    free();

    SDL_Texture *newTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );

    if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			width = loadedSurface->w;
			height = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
    } 
    texture = newTexture;
	return texture != NULL;
}

void Texture::render( int x, int y, int w, int h, SDL_Rect *clip )
{
    //Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, w, h };

	// //Set clip rendering dimensions
	// if( clip != NULL )
	// {
	// 	renderQuad.w = clip->w;
	// 	renderQuad.h = clip->h;
	// }

	//Render to screen
	SDL_RenderCopy( renderer, texture, clip, &renderQuad );
}

int Texture::getWidth()
{
    return width;
}

int Texture::getHeight()
{
    return height;
}