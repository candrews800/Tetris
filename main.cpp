//Brick Layer
//by Clinton Andrews

//Headers
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <string>
#include <sstream>
#include <vector>
#include "Timer.h"
#include "Block.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iostream>

//Screen attributes
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;

//The frame rate
const int FRAMES_PER_SECOND = 60;

Block grid[10][20];
Block active[4];
Block temp[4];

int activeghost_x[4];
int activeghost_y[4];
int lastghost_x[4];
int lastghost_y[4];

bool activeblock = false;
bool pause = false;
bool game = false;
bool intro = true;
bool quit = false;
int blocktype = 0;
int nextblock = 0;
int orientation = 0;
int under = 0;
int oldunder = 17;
int level = 1;
int score = 0;
int lines = 0;
int zero = 0;
int one = 0;
int two = 0;
int three = 0;
int four = 0;
int five = 0;
int six = 0;
int lastzero = 0;
int lastMove = 0;
int autoMove = 0;
int lastRotate = 0;
int lastPause = 0;
int levelchange = 0;
int linescleared = 0;


//The surfaces
SDL_Surface *screen = NULL;
SDL_Surface *gamebackground = NULL;
SDL_Surface *introscreen = NULL;
SDL_Surface *scoretext = NULL;
SDL_Surface *leveltext = NULL;
SDL_Surface *maxtext = NULL;
SDL_Surface *zerotext = NULL;
SDL_Surface *onetext = NULL;
SDL_Surface *twotext = NULL;
SDL_Surface *threetext = NULL;
SDL_Surface *fourtext = NULL;
SDL_Surface *fivetext = NULL;
SDL_Surface *sixtext = NULL;
SDL_Surface *pausescreen = NULL;
SDL_Surface *endscreen = NULL;

Mix_Music *music = NULL;

Mix_Chunk *lineclear = NULL;
Mix_Chunk *tetris = NULL;
Mix_Chunk *move = NULL;

//The font that's going to be used
TTF_Font *font = NULL;

//The color of the font
SDL_Color textColor = { 255, 255, 255 };

//The event structure
SDL_Event event;

//SDL Functions
SDL_Surface *load_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

	//Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        return false;    
    }

	//Initialize SDL_mixer
    if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
    {
        return false;    
    }

	gamebackground=load_image("game_background.png");

    //Set the window caption
    SDL_WM_SetCaption( "Brick Layer by Clinton Andrews", NULL );

	font = TTF_OpenFont( "data.ttf", 28 );

	lineclear = Mix_LoadWAV("lineclear.wav");
	tetris = Mix_LoadWAV("tetris.wav");
	move = Mix_LoadWAV("move.wav");

	music = Mix_LoadMUS( "music.mid" );

	for(int i=0;i<10;i++)
	{
		for(int j=0;j<20;j++)
		{
			grid[i][j].x=275+i*25;
			grid[i][j].y=50+j*25;
			grid[i][j].image=load_image("blank.png");
		}
	}

    //If everything initialized fine
    return true;
}

void new_game();
void intro_screen();
void main_game();
void game_over();
void update_grid();
void draw_grid();
void handle_input(int direction);
void rotate(int direction);
void new_block();
void fall();
void full_line();
void clear_line(int line);
int random();
void draw_next();
void draw_score();
void draw_level();
void draw_stats();
void max_score();
void draw_ghost();

int main( int argc, char* args[] )
{

	Timer fps;

	nextblock = random()%7;

    if( init() == false )
    {
        return 1;
    }
	
	//Game Loop
	while ( quit == false )
	{
		if( Mix_PlayingMusic() == 0 )
		{
			Mix_PlayMusic( music, -1 );
		}
		//Handle Input
		while( SDL_PollEvent( &event ) )
        {

            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
		}
		
		if(intro==true)
		{
			intro_screen();
		}

		Uint8 *keystates = SDL_GetKeyState( NULL );
		if( keystates[ SDLK_SPACE ]  && lastPause + 250 < SDL_GetTicks() && pause == true && intro == false)
		{
			pause=false;
			lastPause=SDL_GetTicks();
		}
		if(pause == false && game == true && intro == false)
		{
			main_game();
		}
		if(game == false && intro == false)
		{
			game_over();
		}

		SDL_Flip( screen );

		//Cap the frame rate
		if( fps.get_ticks() < 1000 / FRAMES_PER_SECOND )
		{
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps.get_ticks() );
		}
	}


    //Clean up
    SDL_Quit();

    return 0;
}

void new_game()
{
	//Set all variables to their original state

	game=true;

	for(int i=0;i<10;i++)
	{
		for(int j=0;j<20;j++)
		{
			grid[i][j].image=load_image("blank.png");
			grid[i][j].active=false;
			grid[i][j].occupied=false;
		}
	}

activeblock = false;
pause = false;
game = true;
quit = false;
blocktype = 0;
nextblock = 0;
orientation = 0;
level = 1;
score = 0;
lines = 0;
zero = 0;
one = 0;
two = 0;
three = 0;
four = 0;
five = 0;
six = 0;
lastzero = 0;
lastMove = 0;
autoMove = 0;
lastRotate = 0;
lastPause = 0;
}

void intro_screen()
{
	introscreen=load_image("intro.png");
	apply_surface(0,0,introscreen,screen);

	std::string s;
	std::stringstream out;
	out << level;
	s = out.str();

	leveltext = TTF_RenderText_Solid( font, s.c_str(), textColor );
	apply_surface(595,403,leveltext,screen);

	if( event.type == SDL_MOUSEBUTTONDOWN )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            //Get the mouse offsets
            int x = event.button.x;
            int y = event.button.y;
        
			if( ( x > 110 ) && ( x < 260 ) && ( y > 230 ) && ( y < 490 ) )
			{
				quit=true;
			}
			//if the mouse is over quit
			else if( ( x > 295 ) && ( x < 535) && ( y > 360 ) && ( y < 480 ) )
			{
				game=true;
				intro=false;
			}
			else if( ( x > 580 ) && ( x < 625) && ( y > 360 ) && ( y < 400 ) && levelchange + 100 < SDL_GetTicks())
			{
				level+=1;
				if(level>50)
				{
					level=50;
				}
				levelchange=SDL_GetTicks();
			}
			else if( ( x > 580 ) && ( x < 625) && ( y > 435 ) && ( y < 480 ) && levelchange + 100 < SDL_GetTicks() )
			{
				level-=1;
				if(level<1)
				{
					
					level=1;
				}
				levelchange=SDL_GetTicks();
			}
        }
    }
}

void main_game()
{
	apply_surface(0,0,gamebackground,screen);
	Uint8 *keystates = SDL_GetKeyState( NULL );
	if( keystates[ SDLK_LEFT ] && lastMove + 150 < SDL_GetTicks())
	{
		handle_input(-1);
		lastMove=SDL_GetTicks();
	}
	if( keystates[ SDLK_RIGHT ] && lastMove + 150 < SDL_GetTicks())
	{
		handle_input(1);
		lastMove=SDL_GetTicks();
	}
	if( keystates[ SDLK_DOWN ] && lastMove + 100 < SDL_GetTicks())
	{
		fall();
		autoMove=SDL_GetTicks();
	}
	if( keystates[ SDLK_q ] && lastRotate + 150 < SDL_GetTicks())
	{
		rotate(0);
		lastRotate=SDL_GetTicks();
	}
	if( keystates[ SDLK_UP ] && lastRotate + 150 < SDL_GetTicks())
	{
		rotate(0);
		lastRotate=SDL_GetTicks();
	}
	if( keystates[ SDLK_w ] && lastRotate + 150 < SDL_GetTicks())
	{
		rotate(1);
		lastRotate=SDL_GetTicks();
	}

	//Game Logic
	//Auto Move Down
	if( (autoMove + 500 - level*8) < SDL_GetTicks())
	{
		fall();
		autoMove=SDL_GetTicks();
	}

	//Check for full lines
	full_line();
		
	//Check if there needs to be a new block -- this has to be after full_line or full_line will shift players piece down as it clears rows
	if(activeblock==false)
	{
		new_block();
	}
		

	//Update + Draw Screen
	draw_grid();
	draw_next();
	draw_level();
	draw_score();
	draw_stats();
	max_score();
	draw_ghost();

	if( keystates[ SDLK_SPACE ]  && lastPause + 250 < SDL_GetTicks() )
	{
		pause=true;
		lastPause=SDL_GetTicks();
		pausescreen=load_image("pause.png");
		apply_surface(250,225,pausescreen,screen);
	}
}

void game_over()
{
	endscreen=load_image("gameover.png");
	if( event.type == SDL_MOUSEMOTION )
    {
        //Get the mouse offsets
        int x = event.motion.x;
        int y = event.motion.y;
        
        //If the mouse is over try again
        if( ( x > 270 ) && ( x < 390 ) && ( y > 305 ) && ( y < 350 ) )
        {
              endscreen=load_image("tryagain.png");
        }
		//if the mouse is over quit
		else if( ( x > 410 ) && ( x < 530) && ( y > 305 ) && ( y < 350 ) )
        {
			endscreen=load_image("quit.png");
        }
        //If not
        else
        {
            endscreen=load_image("gameover.png");
        }    
    }

	apply_surface(250,225,endscreen,screen);

	if( event.type == SDL_MOUSEBUTTONDOWN )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            //Get the mouse offsets
            int x = event.button.x;
            int y = event.button.y;
        
			if( ( x > 270 ) && ( x < 390 ) && ( y > 305 ) && ( y < 350 ) )
			{
				new_game();
			}
			//if the mouse is over quit
			else if( ( x > 410 ) && ( x < 530) && ( y > 305 ) && ( y < 350 ) )
			{
				quit=true;
			}
        }
    }

}

void draw_grid(){
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<20;j++)
		{
			apply_surface(grid[i][j].x,grid[i][j].y,grid[i][j].image,screen);
		}
	}
}

void draw_next()
{
	SDL_Surface *next = NULL;

	switch(nextblock)
	{
	case 0:
		next=load_image("0.png");
		apply_surface(640,120,next,screen);
		break;
	case 1:
		next=load_image("1.png");
		apply_surface(640,120,next,screen);
		break;
	case 2:
		next=load_image("2.png");
		apply_surface(640,120,next,screen);
		break;
	case 3:
		next=load_image("3.png");
		apply_surface(640,120,next,screen);
		break;
	case 4:
		next=load_image("4.png");
		apply_surface(640,120,next,screen);
		break;
	case 5:
		next=load_image("5.png");
		apply_surface(640,120,next,screen);
		break;
	case 6:
		next=load_image("6.png");
		apply_surface(640,120,next,screen);
		break;
	}

}

void draw_score()
{
	std::string s;
	std::stringstream out;
	out << score;
	s = out.str();


	if(score==0)
	{
		s="0000000"+s;
	}

	else if(score<100)
	{
		s="000000"+s;
	}
	else if(score<1000)
	{
		s="00000"+s;
	}
	else if(score<10000 )
	{
		s="0000"+s;
	}
	else if(score<100000)
	{
		s="000"+s;
	}
	else if(score<1000000)
	{
		s="00"+s;
	}
	else if(score<10000000)
	{
		s="0"+s;
	}

	scoretext = TTF_RenderText_Solid( font, s.c_str(), textColor );
	apply_surface(605,275,scoretext,screen);
}

void draw_level()
{
	std::string s;
	std::stringstream out;
	out << level;
	s = out.str();

	leveltext = TTF_RenderText_Solid( font, s.c_str(), textColor );
	apply_surface(650,420,leveltext,screen);
}

void draw_stats()
{
	std::string s;
	std::stringstream outzero;
	std::stringstream outone;
	std::stringstream outtwo;
	std::stringstream outthree;
	std::stringstream outfour;
	std::stringstream outfive;
	std::stringstream outsix;

	outzero << zero;
	s=outzero.str();
	zerotext=TTF_RenderText_Solid( font, s.c_str(), textColor );
	apply_surface(170,255,zerotext,screen);

	outone << one;
	onetext=TTF_RenderText_Solid( font, outone.str().c_str(), textColor );
	apply_surface(170,300,onetext,screen);

	outtwo << two;
	twotext=TTF_RenderText_Solid( font, outtwo.str().c_str(), textColor );
	apply_surface(170,340,twotext,screen);

	outthree << three;
	threetext=TTF_RenderText_Solid( font, outthree.str().c_str(), textColor );
	apply_surface(170,375,threetext,screen);

	outfour << four;
	fourtext=TTF_RenderText_Solid( font, outfour.str().c_str(), textColor );
	apply_surface(170,410,fourtext,screen);

	outfive << five;
	fivetext=TTF_RenderText_Solid( font, outfive.str().c_str(), textColor );
	apply_surface(170,450,fivetext,screen);

	outsix << six;
	sixtext=TTF_RenderText_Solid( font, outsix.str().c_str(), textColor );
	apply_surface(170,480,sixtext,screen);
	
}

void draw_ghost()
{
	//TBD
}

void max_score()
{
	std::string smaxscore="0";
	std::string file="maxscore";
	std::ifstream load;
	load.open(file.c_str());
	
	load >> smaxscore;
	
	load.close();

	int maxscore = atoi(smaxscore.c_str());
	

	if(score>maxscore)
	{
		maxscore=score;
	}

	std::ofstream save(file.c_str());
	save << maxscore;
		
	std::string s;
	std::stringstream out;
	out << maxscore;
	s = out.str();

	if(maxscore==0)
	{
		s="000000"+s;
	}

	else if(maxscore<100)
	{
		s="000000"+s;
	}
	else if(maxscore<1000)
	{
		s="00000"+s;
	}
	else if(maxscore<10000 )
	{
		s="0000"+s;
	}
	else if(maxscore<100000)
	{
		s="000"+s;
	}
	else if(maxscore<1000000)
	{
		s="00"+s;
	}
	else if(maxscore<10000000)
	{
		s="0"+s;
	}

	maxtext = TTF_RenderText_Solid( font, s.c_str(), textColor );
	apply_surface(607,532,maxtext,screen);
		
	save.close();
	
}

void handle_input(int direction)
{
	bool openspace = true;

	//Find Active Pieces, Determine If the Move Can Be Made, Perform If it can
	for(int i=0;i<4;i++)
	{
		if(grid[active[i].x+direction][active[i].y].occupied == true || (direction == -1 && active[i].x==0) || (direction == 1 && active[i].x==9))
		{
			openspace = false;
		}
	}

	if(openspace)
	{
		for(int i=0;i<4;i++)
		{
			temp[i].image = grid[active[i].x][active[i].y].image;
			//put block in temp
			//clear all cells
			//place block in new spot
		}
		for(int i=0;i<4;i++)
		{
			grid[active[i].x][active[i].y].image=load_image("blank.png");
			grid[active[i].x][active[i].y].active = false;
		}
		for(int i=0;i<4;i++)
		{
			grid[active[i].x+direction][active[i].y].image = temp[i].image;
			grid[active[i].x+direction][active[i].y].active = true;
			active[i].x+=direction;
		}

		Mix_PlayChannel( -1, move, 0 );
	}
}

void rotate(int direction)
{

	bool space = true;


	//For all: Check to see if it can be rotated
	//If it can be rotated, previous spots are inactive and blank then set new spots

	switch(blocktype)
	{
	case 0:
		if(direction == 0)
		{
			if(orientation==0)
			{
				if(active[0].x<9 && active[0].x>1)
				{
					if( grid[active[1].x-2][active[1].y].occupied==false && grid[active[1].x-1][active[1].y].occupied==false && grid[active[1].x+1][active[1].y].occupied==false )
					{
						temp[0].image = grid[active[1].x][active[0].y].image;

						grid[active[1].x][active[0].y].active = false;
						grid[active[1].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[1].x][active[2].y].active = false;
						grid[active[1].x][active[2].y].image = load_image("blank.png");
						grid[active[1].x][active[3].y].active = false;
						grid[active[1].x][active[3].y].image = load_image("blank.png");

						grid[active[1].x-2][active[1].y].active = true;
						grid[active[1].x-2][active[1].y].image = temp[0].image;
						grid[active[1].x-1][active[1].y].active = true;
						grid[active[1].x-1][active[1].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[1].x+1][active[1].y].active = true;
						grid[active[1].x+1][active[1].y].image = temp[0].image;

						active[0].x-=2;
						active[1].x-=1;
						active[3].x+=1;

						active[0].y+=1;
						active[2].y-=1;
						active[3].y-=2;

						orientation = 1;
					}
				}
			}
			else if(orientation==1)
			{
				if(active[0].y<18)
				{
					if( grid[active[2].x][active[2].y+1].occupied==false && grid[active[2].x][active[2].y-1].occupied==false && grid[active[2].x][active[2].y-2].occupied==false )
					{
						temp[0].image = grid[active[1].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						grid[active[2].x][active[2].y-1].active = true;
						grid[active[2].x][active[2].y-1].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[2].x][active[2].y+1].active = true;
						grid[active[2].x][active[2].y+1].image = temp[0].image;
						grid[active[2].x][active[2].y+2].active = true;
						grid[active[2].x][active[2].y+2].image = temp[0].image;

						active[0].x+=2;
						active[1].x+=1;
						active[3].x-=1;

						active[0].y-=1;
						active[2].y+=1;
						active[3].y+=2;

						orientation = 0;
					}
				}
			}
		}
		else if(direction == 1)
		{
			if(orientation==0)
			{
				if(active[0].x<9 && active[0].x>1)
				{
					if( grid[active[1].x-2][active[1].y].occupied==false && grid[active[1].x-1][active[1].y].occupied==false && grid[active[1].x+1][active[1].y].occupied==false )
					{
						temp[0].image = grid[active[1].x][active[0].y].image;

						grid[active[1].x][active[0].y].active = false;
						grid[active[1].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[1].x][active[2].y].active = false;
						grid[active[1].x][active[2].y].image = load_image("blank.png");
						grid[active[1].x][active[3].y].active = false;
						grid[active[1].x][active[3].y].image = load_image("blank.png");

						grid[active[1].x-2][active[1].y].active = true;
						grid[active[1].x-2][active[1].y].image = temp[0].image;
						grid[active[1].x-1][active[1].y].active = true;
						grid[active[1].x-1][active[1].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[1].x+1][active[1].y].active = true;
						grid[active[1].x+1][active[1].y].image = temp[0].image;

						active[0].x-=2;
						active[1].x-=1;
						active[3].x+=1;

						active[0].y+=1;
						active[2].y-=1;
						active[3].y-=2;

						orientation = 1;
					}
				}
			}
			else if(orientation==1)
			{
				if(active[0].y<18)
				{
					if( grid[active[2].x][active[2].y+1].occupied==false && grid[active[2].x][active[2].y-1].occupied==false && grid[active[2].x][active[2].y-2].occupied==false )
					{
						temp[0].image = grid[active[1].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						grid[active[2].x][active[2].y-1].active = true;
						grid[active[2].x][active[2].y-1].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[2].x][active[2].y+1].active = true;
						grid[active[2].x][active[2].y+1].image = temp[0].image;
						grid[active[2].x][active[2].y+2].active = true;
						grid[active[2].x][active[2].y+2].image = temp[0].image;

						active[0].x+=2;
						active[1].x+=1;
						active[3].x-=1;

						active[0].y-=1;
						active[2].y+=1;
						active[3].y+=2;

						orientation = 0;
					}
				}
			}
		}
		break;

	case 1:

		if( grid[active[2].x-1][active[2].y-1].occupied == true || grid[active[2].x][active[2].y-1].occupied == true || grid[active[2].x+1][active[2].y-1].occupied == true ||
			grid[active[2].x-1][active[2].y].occupied == true || grid[active[2].x][active[2].y].occupied == true || grid[active[2].x+1][active[2].y].occupied == true ||
			grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x+1][active[2].y+1].occupied == true)
		{
			space = false;
		}
		if(space)
		{
			if(direction == 0)
			{
				if(orientation==0)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x-1;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y-1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 3;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x+1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;
					
					active[0].y=active[2].y-1;
					active[1].y=active[2].y-1;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
				else if(orientation==2)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x+1;
						active[1].x=active[2].x+1;
						active[3].x=active[2].x-1;

						active[0].y=active[2].y+1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==3)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x-1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;
					
					active[0].y=active[2].y+1;
					active[1].y=active[2].y+1;
					active[3].y=active[2].y-1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 2;
				}
			}
			else if(direction == 1)
			{
				if(orientation==0)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x+1;
						active[1].x=active[2].x+1;
						active[3].x=active[2].x-1;

						active[0].y=active[2].y+1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x-1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;
					
					active[0].y=active[2].y+1;
					active[1].y=active[2].y+1;
					active[3].y=active[2].y-1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 2;
				}
				else if(orientation==2)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x-1;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y-1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 3;
					}
				}
				else if(orientation==3)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x+1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;
					
					active[0].y=active[2].y-1;
					active[1].y=active[2].y-1;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
			}
		}
		break;
	case 2:

		if( grid[active[2].x-1][active[2].y-1].occupied == true || grid[active[2].x][active[2].y-1].occupied == true || grid[active[2].x+1][active[2].y-1].occupied == true ||
			grid[active[2].x-1][active[2].y].occupied == true || grid[active[2].x][active[2].y].occupied == true || grid[active[2].x+1][active[2].y].occupied == true ||
			grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x+1][active[2].y+1].occupied == true)
		{
			space = false;
		}
		if(space)
		{
			if(direction == 0)
			{
				if(orientation==0)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y-1;
						active[1].y=active[2].y-1;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x+1;
					active[1].x=active[2].x+1;
					active[3].x=active[2].x;

					active[0].y=active[2].y-1;
					active[1].y=active[2].y;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
			}
			else if(direction == 1)
			{
				if(orientation==0)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y-1;
						active[1].y=active[2].y-1;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x+1;
					active[1].x=active[2].x+1;
					active[3].x=active[2].x;

					active[0].y=active[2].y-1;
					active[1].y=active[2].y;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
			}
		}
		break;

	case 4:

		if( grid[active[1].x-1][active[1].y-1].occupied == true || grid[active[1].x][active[1].y-1].occupied == true || grid[active[1].x+1][active[1].y-1].occupied == true ||
			grid[active[1].x-1][active[1].y].occupied == true || grid[active[1].x][active[1].y].occupied == true || grid[active[1].x+1][active[1].y].occupied == true ||
			grid[active[1].x-1][active[1].y+1].occupied == true || grid[active[1].x-1][active[1].y+1].occupied == true || grid[active[1].x+1][active[1].y+1].occupied == true)
		{
			space = false;
		}
		if(space)
		{
			if(direction == 0)
			{
				if(orientation==0)
				{
					if(active[1].x>=1 && active[1].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[1].x-1;
						active[2].x=active[1].x;
						active[3].x=active[1].x+1;

						active[0].y=active[1].y;
						active[2].y=active[1].y-1;
						active[3].y=active[1].y-1;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[1].x;
					active[2].x=active[1].x+1;
					active[3].x=active[1].x+1;

					active[0].y=active[1].y-1;
					active[2].y=active[1].y;
					active[3].y=active[1].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
			}
			else if(direction == 1)
			{
				if(orientation==0)
				{
					if(active[1].x>=1 && active[1].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[1].x-1;
						active[2].x=active[1].x;
						active[3].x=active[1].x+1;

						active[0].y=active[1].y;
						active[2].y=active[1].y-1;
						active[3].y=active[1].y-1;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[1].x;
					active[2].x=active[1].x+1;
					active[3].x=active[1].x+1;

					active[0].y=active[1].y-1;
					active[2].y=active[1].y;
					active[3].y=active[1].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
			}
		}
		break;
	case 5:

		if( grid[active[2].x-1][active[2].y-1].occupied == true || grid[active[2].x][active[2].y-1].occupied == true || grid[active[2].x+1][active[2].y-1].occupied == true ||
			grid[active[2].x-1][active[2].y].occupied == true || grid[active[2].x][active[2].y].occupied == true || grid[active[2].x+1][active[2].y].occupied == true ||
			grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x+1][active[2].y+1].occupied == true)
		{
			space = false;
		}
		if(space)
		{
			if(direction == 0)
			{
				if(orientation==0)
				{
					if(active[1].x>=1 && active[1].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x-1;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y+1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x+1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;

					active[0].y=active[2].y+1;
					active[1].y=active[2].y+1;
					active[3].y=active[2].y-1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 2;
				}
				else if(orientation==2)
				{
					if(active[1].x>=1 && active[1].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x+1;
						active[1].x=active[2].x+1;
						active[3].x=active[2].x-1;

						active[0].y=active[2].y-1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 3;
					}
				}
				else if(orientation==3)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x-1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;

					active[0].y=active[2].y-1;
					active[1].y=active[2].y-1;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
			}
			else if(direction == 1)
			{
				if(orientation==0)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x+1;
						active[1].x=active[2].x+1;
						active[3].x=active[2].x-1;

						active[0].y=active[2].y-1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 3;
					}
				}
				else if(orientation==1)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x-1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;

					active[0].y=active[2].y-1;
					active[1].y=active[2].y-1;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 0;
				}
				else if(orientation==2)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x-1;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y+1;
						active[1].y=active[2].y;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation = 1;
					}
				}
				else if(orientation==3)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x+1;
					active[1].x=active[2].x;
					active[3].x=active[2].x;

					active[0].y=active[2].y+1;
					active[1].y=active[2].y+1;
					active[3].y=active[2].y-1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation = 2;
				}
			}
		}
		break;
	case 6:

		if( grid[active[2].x-1][active[2].y-1].occupied == true || grid[active[2].x][active[2].y-1].occupied == true || grid[active[2].x+1][active[2].y-1].occupied == true ||
			grid[active[2].x-1][active[2].y].occupied == true || grid[active[2].x][active[2].y].occupied == true || grid[active[2].x+1][active[2].y].occupied == true ||
			grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x-1][active[2].y+1].occupied == true || grid[active[2].x+1][active[2].y+1].occupied == true)
		{
			space = false;
		}
		if(space)
		{
			if(direction == 0)
			{
				if(orientation==0)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x;
					active[1].x=active[2].x-1;
					active[3].x=active[2].x;

					active[0].y=active[2].y+1;
					active[1].y=active[2].y;
					active[3].y=active[2].y-1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation=3;
				}
				else if(orientation==1)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
							grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y;
						active[1].y=active[2].y-1;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation=0;
					}
				}
				else if(orientation==2)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x;
					active[1].x=active[2].x+1;
					active[3].x=active[2].x;

					active[0].y=active[2].y-1;
					active[1].y=active[2].y;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation=1;
				}
				else if(orientation==3)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x+1;
						active[1].x=active[2].x;
						active[3].x=active[2].x-1;

						active[0].y=active[2].y;
						active[1].y=active[2].y+1;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation=2;
					}
				}
			}
			else if(direction == 1)
			{
				if(orientation==0)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x;
					active[1].x=active[2].x+1;
					active[3].x=active[2].x;

					active[0].y=active[2].y-1;
					active[1].y=active[2].y;
					active[3].y=active[2].y+1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation=1;
				}
				else if(orientation==1)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x+1;
						active[1].x=active[2].x;
						active[3].x=active[2].x-1;

						active[0].y=active[2].y;
						active[1].y=active[2].y+1;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation=2;
					}
				}
				else if(orientation==2)
				{
					temp[0].image = grid[active[0].x][active[0].y].image;

					grid[active[0].x][active[0].y].active = false;
					grid[active[0].x][active[0].y].image = load_image("blank.png");
					grid[active[1].x][active[1].y].active = false;
					grid[active[1].x][active[1].y].image = load_image("blank.png");
					grid[active[2].x][active[2].y].active = false;
					grid[active[2].x][active[2].y].image = load_image("blank.png");
					grid[active[3].x][active[3].y].active = false;
					grid[active[3].x][active[3].y].image = load_image("blank.png");

					active[0].x=active[2].x;
					active[1].x=active[2].x-1;
					active[3].x=active[2].x;

					active[0].y=active[2].y+1;
					active[1].y=active[2].y;
					active[3].y=active[2].y-1;

					grid[active[0].x][active[0].y].active = true;
					grid[active[0].x][active[0].y].image = temp[0].image;
					grid[active[1].x][active[1].y].active = true;
					grid[active[1].x][active[1].y].image = temp[0].image;
					grid[active[2].x][active[2].y].active = true;
					grid[active[2].x][active[2].y].image = temp[0].image;
					grid[active[3].x][active[3].y].active = true;
					grid[active[3].x][active[3].y].image = temp[0].image;

					orientation=3;
				}
				else if(orientation==3)
				{
					if(active[2].x>=1 && active[2].x<=8)
					{
						temp[0].image = grid[active[0].x][active[0].y].image;

						grid[active[0].x][active[0].y].active = false;
						grid[active[0].x][active[0].y].image = load_image("blank.png");
						grid[active[1].x][active[1].y].active = false;
						grid[active[1].x][active[1].y].image = load_image("blank.png");
						grid[active[2].x][active[2].y].active = false;
						grid[active[2].x][active[2].y].image = load_image("blank.png");
						grid[active[3].x][active[3].y].active = false;
						grid[active[3].x][active[3].y].image = load_image("blank.png");

						active[0].x=active[2].x-1;
						active[1].x=active[2].x;
						active[3].x=active[2].x+1;

						active[0].y=active[2].y;
						active[1].y=active[2].y-1;
						active[3].y=active[2].y;

						grid[active[0].x][active[0].y].active = true;
						grid[active[0].x][active[0].y].image = temp[0].image;
						grid[active[1].x][active[1].y].active = true;
						grid[active[1].x][active[1].y].image = temp[0].image;
						grid[active[2].x][active[2].y].active = true;
						grid[active[2].x][active[2].y].image = temp[0].image;
						grid[active[3].x][active[3].y].active = true;
						grid[active[3].x][active[3].y].image = temp[0].image;

						orientation=0;
					}
				}
			}
		}
		break;
	}
}

void fall()
{
	bool openspace = true;

	//Find Active Pieces, Determine If the Move Can Be Made, Perform If it can
	for(int i=0;i<4;i++)
	{
		if(grid[active[i].x][active[i].y+1].occupied == true || active[i].y==19)
		{
			openspace = false;
		}
	}

	if(openspace)
	{
		temp[0].image=grid[active[0].x][active[0].y].image;

		grid[active[0].x][active[0].y].active = false;
		grid[active[0].x][active[0].y].image = load_image("blank.png");
		grid[active[1].x][active[1].y].active = false;
		grid[active[1].x][active[1].y].image = load_image("blank.png");
		grid[active[2].x][active[2].y].active = false;
		grid[active[2].x][active[2].y].image = load_image("blank.png");
		grid[active[3].x][active[3].y].active = false;
		grid[active[3].x][active[3].y].image = load_image("blank.png");

		active[0].y+=1;
		active[1].y+=1;
		active[2].y+=1;
		active[3].y+=1;

		grid[active[0].x][active[0].y].active = true;
		grid[active[0].x][active[0].y].image = temp[0].image;
		grid[active[1].x][active[1].y].active = true;
		grid[active[1].x][active[1].y].image = temp[0].image;
		grid[active[2].x][active[2].y].active = true;
		grid[active[2].x][active[2].y].image = temp[0].image;
		grid[active[3].x][active[3].y].active = true;
		grid[active[3].x][active[3].y].image = temp[0].image;
	}

	if(!openspace)
	{
		for(int i=0;i<4;i++)
		{
			grid[active[i].x][active[i].y].occupied = true;
		}
		activeblock=false;
	}
}

void full_line()
{
	for(int j=0;j<20;j++)
	{
		bool space = false;
		for(int i=0;i<10;i++)
		{
			if(grid[i][j].occupied==false)
			{
				space=true;
			}
		}
		if(space==false)
		{
			clear_line(j);
		}
	}
}

void clear_line(int line){
	for(int j=line;j>0;j--)
	{
		for(int i=0;i<10;i++)
		{
			grid[i][j].image=grid[i][j-1].image;
			grid[i][j].active=grid[i][j-1].active;
			grid[i][j].occupied=grid[i][j-1].occupied;
		}
	}
	lines++;
	linescleared++;
	
	if(lines>=10)
	{
		lines=0;
		level+=1;
	}
	if(level>50)
	{
		level=50;
	}
}

void new_block()
{
	if(linescleared>=4)
	{
		score+=1200*(level+1);
		//tetris sound
		Mix_PlayChannel( -1, tetris, 0 );
	}
	else if(linescleared==3)
	{
		score+=300*(level+1);
		Mix_PlayChannel( -1, lineclear, 0 );
	}
	else if(linescleared==2)
	{
		score+=100*(level+1);
		Mix_PlayChannel( -1, lineclear, 0 );
	}
	else if(linescleared==1)
	{
		score+=40*(level+1);
		Mix_PlayChannel( -1, lineclear, 0 );
	}
	linescleared=0;
	blocktype=nextblock;
	orientation=0;
	lastzero+=1;
	switch(blocktype)
	{
	//Line
	case 0:
		zero+=1;
		lastzero=0;
		grid[4][0].image=load_image("white.png");
		grid[4][1].image=load_image("white.png");
		grid[4][2].image=load_image("white.png");
		grid[4][3].image=load_image("white.png");

		grid[4][0].active=true;
		grid[4][1].active=true;
		grid[4][2].active=true;
		grid[4][3].active=true;

		active[0].x=4;
		active[0].y=0;
		active[1].x=4;
		active[1].y=1;
		active[2].x=4;
		active[2].y=2;
		active[3].x=4;
		active[3].y=3;

		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;
	
	case 1:
		one+=1;
		grid[4][0].image=load_image("red.png");
		grid[4][1].image=load_image("red.png");
		grid[4][2].image=load_image("red.png");
		grid[5][0].image=load_image("red.png");

		grid[4][0].active=true;
		grid[5][0].active=true;
		grid[4][1].active=true;
		grid[4][2].active=true;

		active[0].x=5;
		active[0].y=0;
		active[1].x=4;
		active[1].y=0;
		active[2].x=4;
		active[2].y=1;
		active[3].x=4;
		active[3].y=2;
		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;

	case 2:
		two+=1;
		grid[5][0].image=load_image("yellow.png");
		grid[5][1].image=load_image("yellow.png");
		grid[4][1].image=load_image("yellow.png");
		grid[4][2].image=load_image("yellow.png");

		grid[5][0].active=true;
		grid[5][1].active=true;
		grid[4][1].active=true;
		grid[4][2].active=true;

		active[0].x=5;
		active[0].y=0;
		active[1].x=5;
		active[1].y=1;
		active[2].x=4;
		active[2].y=1;
		active[3].x=4;
		active[3].y=2;
		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;

	case 3:
		three+=1;
		grid[4][0].image=load_image("green.png");
		grid[5][0].image=load_image("green.png");
		grid[4][1].image=load_image("green.png");
		grid[5][1].image=load_image("green.png");

		grid[4][0].active=true;
		grid[5][0].active=true;
		grid[4][1].active=true;
		grid[5][1].active=true;

		active[0].x=4;
		active[0].y=0;
		active[1].x=5;
		active[1].y=0;
		active[2].x=4;
		active[2].y=1;
		active[3].x=5;
		active[3].y=1;
		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;

	case 4:
		four+=1;
		grid[4][0].image=load_image("blue.png");
		grid[4][1].image=load_image("blue.png");
		grid[5][1].image=load_image("blue.png");
		grid[5][2].image=load_image("blue.png");

		grid[4][0].active=true;
		grid[4][1].active=true;
		grid[5][1].active=true;
		grid[5][2].active=true;

		active[0].x=4;
		active[0].y=0;
		active[1].x=4;
		active[1].y=1;
		active[2].x=5;
		active[2].y=1;
		active[3].x=5;
		active[3].y=2;
		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;
		
	case 5:
		five+=1;
		grid[4][0].image=load_image("orange.png");
		grid[5][0].image=load_image("orange.png");
		grid[5][1].image=load_image("orange.png");
		grid[5][2].image=load_image("orange.png");

		grid[4][0].active=true;
		grid[5][0].active=true;
		grid[5][1].active=true;
		grid[5][2].active=true;

		active[0].x=4;
		active[0].y=0;
		active[1].x=5;
		active[1].y=0;
		active[2].x=5;
		active[2].y=1;
		active[3].x=5;
		active[3].y=2;
		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;
		
	case 6:
		six+=1;
		grid[4][1].image=load_image("purple.png");
		grid[5][0].image=load_image("purple.png");
		grid[5][1].image=load_image("purple.png");
		grid[6][1].image=load_image("purple.png");

		grid[4][1].active=true;
		grid[5][0].active=true;
		grid[5][1].active=true;
		grid[6][1].active=true;

		active[0].x=4;
		active[0].y=1;
		active[1].x=5;
		active[1].y=0;
		active[2].x=5;
		active[2].y=1;
		active[3].x=6;
		active[3].y=1;
		for(int i=0;i<4;i++)
		{
			if(grid[active[i].x][active[i].y].occupied)
			{
				game=false;
			}
		}
		break;
	}

	nextblock=random()%7;
	if(lastzero>10)
	{
		nextblock=0;
	}
	activeblock=true;
}

int random()
{
	srand(time(NULL));
	return rand();
}