//g++ bouncingBall.cpp -lSDL2 -lSDL2_image -lSDL2_ttf -o Bouncing Ball

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

#define PI 3.14159265

using namespace std; 
//Screen dimension constants
const int SCREEN_WIDTH = 200;
const int SCREEN_HEIGHT = 200;

//A circle stucture
struct Circle{
	int x, y;
	int r;
};

//Texture wrapper class
class LTexture{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		void setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		void setAlpha(Uint8 alpha);
		
		//Renders texture at given point
		void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

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

//The ball that will move around on the screen
class Ball{
    public:
		//The dimensions of the ball
		static const int BALL_WIDTH = 20;
		static const int BALL_HEIGHT = 20;

		//Maximum axis velocity of the ball
		static const int BALL_VEL = 1;

		//Initializes the variables
		Ball(int x, int y, int velX, int velY);

		//Moves the ball and checks collision
		void move(int currentBall);

		//Shows the ball on the screen
		void render();

		//Gets collision circle
		Circle& getCollider();

    private:
		//The X and Y offsets of the ball
		int mPosX, mPosY;

		//The velocity of the ball
		int mVelX, mVelY;
		
		//Ball's collision circle
		Circle mCollider;

		//Moves the collision circle relative to the ball's offset
		void shiftColliders();
};

//The application time based timer
class LTimer{
    public:
		//Initializes variables
		LTimer();

		//The various clock actions
		void start();
		void stop();

		//Gets the timer's time
		Uint32 getTicks();

    private:
		//The clock time when the timer started
		Uint32 mStartTicks;

		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;

		//The timer status
		bool mPaused;
		bool mStarted;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Load balls in a vector
void loadBalls(int n);

//Circle/Circle collision detector
bool checkCollision(Circle& a, Circle& b);

//Calculates distance squared between two points
double distance(int x1, int y1, int x2, int y2);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Global Timer
LTimer gTimer;

//Scene textures
LTexture gBallTexture;

//Globally used font
TTF_Font* gFont = NULL;

//Scene textures
LTexture gFPSTextTexture;

//normal
vector<double> normalVector;
double magnitudeNormalVector;
vector<double> unitNormalVector;

//Vectors for the balls and their colliders
vector<Ball> gBalls;
vector<Circle> gColliders;

LTexture::LTexture(){
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture(){
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path){
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
		if( newTexture == NULL ){
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else{
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

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor){
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if(textSurface != NULL){
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if(mTexture == NULL){
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free(){
	//Free texture if it exists
	if(mTexture != NULL){
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue){
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending){
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha(Uint8 alpha){
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip){
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL ){
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth(){
	return mWidth;
}

int LTexture::getHeight(){
	return mHeight;
}

Ball::Ball(int x, int y, int velX, int velY){
    

	//Set collision circle size
	mCollider.r = gBallTexture.getWidth() / 2;

	//Initialize the offsets
    mPosX = x;
    mPosY = y;

    //Initialize the velocity
    mVelX = velX;
    mVelY = velY;

	//Move collider relative to the circle
	shiftColliders();
}

//moves and checks if the object circle collides with the current ball being rendered
void Ball::move(int currentBall){

    //Move the ball left or right
    mPosX += mVelX;
	shiftColliders();

	//Move the ball up or down
    mPosY += mVelY;
	shiftColliders();

    //for every collider in gCollider
    for(int i = 0; i < gColliders.size(); i++){
		//If the ball collided or went too far to the left or right and it is not the current ball
	    if( (i != currentBall) &&  ((mPosX < 0) || (mPosX + BALL_WIDTH > SCREEN_WIDTH) || (checkCollision(mCollider, gColliders[i])))){
	        //Reverse x direction
	        mVelX = -1*mVelX;
			shiftColliders();
	    }
	    //If the ball collided or went too far to the left or right and it is not the current ball
	    if( (i != currentBall) && ((mPosY < 0) || (mPosY + BALL_HEIGHT > SCREEN_HEIGHT) || (checkCollision(mCollider, gColliders[i])))){
	        //Reverse y direction
			mVelY = -1*mVelY;
			shiftColliders();
	    }
	    gColliders.at(currentBall) = gBalls[currentBall].getCollider();
	}
}

void Ball::render(){
    //Show the ball
	gBallTexture.render(mPosX, mPosY);
}

Circle& Ball::getCollider(){
	return mCollider;
}

void Ball::shiftColliders(){
	//Align collider to center of ball
	mCollider.x = mPosX;
	mCollider.y = mPosY;
}

LTimer::LTimer(){
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void LTimer::start(){
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop(){
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

Uint32 LTimer::getTicks(){
	//The actual timer time
	Uint32 time = 0;

    //If the timer is running
    if(mStarted){
        //If the timer is paused
        if(mPaused){
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else{
            //Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool init(){
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init(SDL_INIT_VIDEO ) < 0){
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else{
		//Set texture filtering to linear
		if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow("Bouncing Balls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(gWindow == NULL){
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if(gRenderer == NULL){
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) & imgFlags)){
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
				//Initialize SDL_ttf
				if(TTF_Init() == -1){
					printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia(){
	//Loading success flag
	bool success = true;

	//Load ball texture
	if(!gBallTexture.loadFromFile("ball.bmp")){
		printf("Failed to load ball texture!\n");
		success = false;
	}

	gFont = TTF_OpenFont("consola.ttf", 15);
	if(gFont == NULL){
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}

	return success;
}

void close(){
	//Free loaded images
	gBallTexture.free();
	gFPSTextTexture.free();

	//Free global font
	TTF_CloseFont( gFont );
	gFont = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void loadBalls(int n){
	for(int i = 0; i < n; i++){
		Ball ball_moving(rand()%(SCREEN_WIDTH-Ball::BALL_WIDTH), rand()%(SCREEN_HEIGHT-Ball::BALL_HEIGHT), rand()%5-4, rand()%5-3);
		gBalls.push_back(ball_moving);
		gColliders.push_back(ball_moving.getCollider());
	}
}

bool checkCollision(Circle& a, Circle& b){
	//Calculate total radii/diameter
    int totalRadii = a.r + b.r;

    normalVector.push_back(b.x - a.x);
	normalVector.push_back(b.y - a.y);
	magnitudeNormalVector = sqrt(pow(normalVector[0], 2) + pow(normalVector[1], 2));
	

    //If the distance between the centers of the circles is less than the sum of their radii
    if(magnitudeNormalVector < (totalRadii)){
        //The circles have collided
        return true;
    }
    //If not
    return false;
    normalVector.empty();
}

double distance(int x1, int y1, int x2, int y2){
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return sqrt(pow(deltaX, 2) + pow(deltaY, 2));
}

/*void setVectors(Circle& a, Circle& b){
	
	for(int i = 0; i < 2; i++){
		unitNormalVector.at[i] = normalVector.at[i]/magnitudeNormalVector;
	}
}*/

int main( int argc, char* args[] ){
	//Start up SDL and create window
	if(!init()){
		printf( "Failed to initialize!\n" );
	}
	else{
		//Load media
		if(!loadMedia()){
			printf( "Failed to load media!\n" );
		}
		else{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//Set text color as black
			SDL_Color textColor = {0, 0, 0, 255};

			//In memory text stream
			stringstream timeText;

			//Start global timer
			gTimer.start();

			//The frames per second timer
			LTimer fpsTimer;

			//Start counting frames per second
			int countedFrames = 0;
			fpsTimer.start();

			//Count of balls in screen
			int nBalls = 5;

			//loadBalls in vector gBalls
			loadBalls(nBalls);

			//While application is running
			while(!quit){
				//Handle events on queue
				while(SDL_PollEvent(&e) != 0){
					//User requests quit
					if(e.type == SDL_QUIT){
						quit = true;
					}
				}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				
				//Calculate and correct fps
				float avgFPS = countedFrames/(fpsTimer.getTicks()/1000.f);
				if(avgFPS > 2000000){
					avgFPS = 0;
				}

				//Set text to be rendered
				timeText.str("");
				timeText << "Average Frames Per Second " << avgFPS; 

				//Render text
				if(!gFPSTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor)){
					printf("Unable to render FPS texture!\n");
				}
				gFPSTextTexture.render((SCREEN_WIDTH-gFPSTextTexture.getWidth())/2, 0);

				//Move and render the balls inside the vector gBalls
				for(int i = 0; i < nBalls; i++){
					//bug: Needs to include all the balls inside the gBalls for the move method
					gBalls.at(i).move(i);
					gBalls.at(i).render();
				}

				//Update screen
				SDL_RenderPresent(gRenderer);
				++countedFrames;

			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}