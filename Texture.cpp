#include "Texture.hpp"
#include <SDL_image.h>
using namespace std;
//Stores the tile sprite sheet
Texture tileSpriteSheet;

//Sprite sheet clipping rectangles
SDL_Rect tileSpriteClips[8];

//Stores font
TTF_Font *fontBold;
TTF_Font *fontRegular;

//Used for rendering text
Texture textTexture;

//Ingame background
Texture bgImage;

//Menu background texture
Texture menuBackground;

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

bool Texture::loadText( string text, TTF_Font *font, SDL_Color color )
{
	free();
	SDL_Surface *textSurface = TTF_RenderText_Blended(font, text.c_str(), color );

	if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        texture = SDL_CreateTextureFromSurface( renderer, textSurface );
        if( texture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            width = textSurface->w;
            height = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    
    //Return success
    return texture != NULL;

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

void Texture::setColorMod( Uint8 r, Uint8 g, Uint8 b )
{
    SDL_SetTextureColorMod( texture, r, g, b );
}

void Texture::setAlphaMod( Uint8 a )
{
    SDL_SetTextureAlphaMod( texture, a );
}

void Texture::render( int x, int y, int w, int h, SDL_Rect *clip, int angle )
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
    if ( angle != 0 )
	    SDL_RenderCopyEx( renderer, texture, clip, &renderQuad, angle, NULL, SDL_FLIP_NONE );
    else SDL_RenderCopy( renderer, texture, clip, &renderQuad );
}

int Texture::getWidth()
{
    return width;
}

int Texture::getHeight()
{
    return height;
}