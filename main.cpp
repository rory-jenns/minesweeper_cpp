/*This source code copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

#include <MineBoard.cpp>

#define IMAGE_SRC "../MS-Texture/png/cells/ChocolateSweeper"


//Button constants
const int BUTTON_WIDTH = 16;
const int BUTTON_HEIGHT = 16;
const int TOTAL_BUTTONS = 16;

// mine sprites
const int MINESPRITE_SIZE = 16;
const int NUM_SPRITES = 16;
const int SCALING = 2;

//Screen dimension constants

const int NUM_WIDTH = 10;
const int NUM_HEIGHT = 10;
const int NUM_MINES = 10;

const int SCREEN_PADDING = 20;

const int BOARD_WIDTH = MINESPRITE_SIZE*SCALING*NUM_WIDTH;
const int BOARD_HEIGHT = MINESPRITE_SIZE*SCALING*NUM_HEIGHT;

const int SCREEN_WIDTH = SCREEN_PADDING*2+BOARD_WIDTH;
const int SCREEN_HEIGHT =  SCREEN_PADDING*2+BOARD_HEIGHT;

enum SpriteStates
{
    ADJ_ONE, ADJ_TWO, ADJ_THREE, ADJ_FOUR, 
    ADJ_FIVE, ADJ_SIX, ADJ_SEVEN, ADJ_EIGHT,
    EMPTY, COVERED, FLAGGED, FALSE_MINE, 
    Q, Q_COVERED, MINE, MINE_FAIL
};

int sprite_mapping[12] = {
	EMPTY, ADJ_ONE, ADJ_TWO, ADJ_THREE, ADJ_FOUR, 
	ADJ_FIVE, ADJ_SIX, ADJ_SEVEN, ADJ_EIGHT,
	FLAGGED, COVERED, MINE
};

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#if defined(SDL_TTF_MAJOR_VERSION)
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif
		
		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE , float x_scale = 1.0, float y_scale = 1.0);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Mouse button sprites
SDL_Rect gTileSpriteClips[ TOTAL_BUTTONS ];
LTexture gButtonSpriteSheetTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface == NULL )
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip , float x_scale, float y_scale)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

    renderQuad.w = renderQuad.w * x_scale;
    renderQuad.h = renderQuad.h * y_scale;

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}


int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}


bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load sprites
	if( !gButtonSpriteSheetTexture.loadFromFile( "../WinmineXP.png" ) )
	{
		printf( "Failed to load button sprite texture!\n" );
		success = false;
	}
	else
	{
        // Set sprites
		for( int i = 0; i < NUM_SPRITES; ++i )
		{
			gTileSpriteClips[ i ].x = (i % 4)*16;
			gTileSpriteClips[ i ].y = (i / 4) * 16;
			gTileSpriteClips[ i ].w = BUTTON_WIDTH;
			gTileSpriteClips[ i ].h = BUTTON_HEIGHT;
		}
    }

	return success;
}

void close()
{
	//Free loaded images
	gButtonSpriteSheetTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}


void drawBoard(SDL_Renderer* gRenderer , MineBoard* mineboard) {

	// printf("draw board");
    int dest_sprite_size = MINESPRITE_SIZE*SCALING;

    SDL_Rect boardVP;
    boardVP.x = SCREEN_PADDING;
    boardVP.y = SCREEN_PADDING;
    boardVP.w = BOARD_WIDTH;
    boardVP.h = BOARD_HEIGHT;


    SDL_RenderSetViewport( gRenderer, &boardVP );

    int x, y, square;
    for (y=0; y<mineboard->getHeight();y++){
        for (x=0; x<mineboard->getWidth();x++){
            square = mineboard->showSquare(x,y);

            gButtonSpriteSheetTexture.render( x*dest_sprite_size, y*dest_sprite_size, &gTileSpriteClips[ sprite_mapping[square] ], 0.0, NULL, SDL_FLIP_NONE, 2.0, 2.0);
        }
    }
}

int main( int argc, char* args[] )
{
	bool play=true, win=false,lose=false;
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

            MineBoard mineboard(NUM_WIDTH, NUM_HEIGHT, NUM_MINES);

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT)
					{
						// TODO add quit by ESC key
						quit = true;
					}

					if(e.type == SDL_MOUSEBUTTONDOWN
					)
					{
						printf("mousedown!\n");
					    //Get mouse position
					    int x, y;
					    SDL_GetMouseState( &x, &y );

						// bounds
						if (
							!(x < SCREEN_PADDING 
								|| x >= SCREEN_PADDING + BOARD_WIDTH 
								|| y < SCREEN_PADDING 
								|| y >= SCREEN_PADDING + BOARD_HEIGHT
							))
						{

							// inside
							int tilex, tiley;
							tilex = (x - SCREEN_PADDING) / (SCALING*MINESPRITE_SIZE);
							tiley = (y - SCREEN_PADDING) / (SCALING*MINESPRITE_SIZE);
							
							if (play) {
								if (e.button.button == SDL_BUTTON_LEFT) {								
									mineboard.sweep(tilex,tiley);
								}
								if (e.button.button == SDL_BUTTON_RIGHT) {
									mineboard.flag(tilex, tiley);
								}
							}
							else {
								if (e.button.button == SDL_BUTTON_LEFT) {								
									mineboard.reset();
									play = true;
								}
							}
						}

					}
				}

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

                drawBoard(gRenderer, &mineboard);

				//Update screen
				SDL_RenderPresent( gRenderer );

				// end logic
				lose = mineboard.check_lose();
				win = mineboard.check_win();
				if (win || lose) {
					// TODO add text announcement on win/lose
					// w/ "click to play again"
					play = false;
					mineboard.uncover_board();
				}
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
