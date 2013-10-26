#include "SDL.h"

class Block{

public:
	int x, y;
	bool active;
	bool occupied;

	SDL_Surface *image;

	Block();
};