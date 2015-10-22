#ifdef __cplusplus
	#include <cstdlib>
#else
	#include <stdlib.h>
#endif

#include <SDL/SDL.h>
#include <math.h>

SDL_Surface* screen;

struct Color {
	float r, g, b;

	Color(float r, float g, float b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	inline static Uint32 quantize(float f)
	{
		if (f > 1) f = 1;
		if (f < 0) f = 0;
		return Uint32(f * 255);
	}

	Uint32 toRGB32(void) const
	{
		return quantize(b) | (quantize(g) << 8) | (quantize(r) << 16);
	}
};

inline void putPixel(int x, int y, Color color)
{
	Uint32 pixel = color.toRGB32();

	if (x < 0 || x >= screen->w || y < 0 || y >= screen->h)
		return;

	Uint8* framebuffer = (Uint8*) screen->pixels;
	Uint8* row = framebuffer + y * screen->pitch;

	Uint32* rowPixels = (Uint32*) row;
	rowPixels[x] = pixel;
}

float toRadians(float angle)
{
	return angle/ 180.0 * 3.141592;
}

void render()
{
	int centerX = 100;
	int centerY = 200;
	// p = (centerX, centerY) + (cos(angle)*r, sin(angle)*r)
	for (float angle = 0; angle < 360; angle += 0.5) {
		for (int radius = 0; radius < 50; radius++)
		putPixel(centerX + cos(toRadians(angle)) * radius,
				 centerY + sin(toRadians(angle)) * radius,
				 Color(1, 1 , radius/50.0));
	}
}

int main ( int argc, char** argv )
{
	// initialize SDL video
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "Unable to init SDL: %s\n", SDL_GetError() );
		return 1;
	}

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	// create a new window
	screen = SDL_SetVideoMode(640, 480, 32, 0);
	if ( !screen )
	{
		printf("Unable to set 640x480 video: %s\n", SDL_GetError());
		return 1;
	}

	// do da work:
	render();

	// program main loop
	bool done = false;
	while (!done)
	{
		// message processing loop
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// check for messages
			switch (event.type)
			{
				// exit if the window is closed
			case SDL_QUIT:
				done = true;
				break;

				// check for keypresses
			case SDL_KEYDOWN:
				{
					// exit if ESCAPE is pressed
					if (event.key.keysym.sym == SDLK_ESCAPE)
						done = true;
					break;
				}
			} // end switch
		} // end of message processing

		// DRAWING STARTS HERE

		SDL_Flip(screen);
	} // end main loop

	// all is well ;)
	printf("Exited cleanly\n");
	return 0;
}
