#include <SDL/SDL.h>
#include <math.h>
#include "util.h"
#include "sdl.h"
#include "color.h"

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

void render()
{
	for (int y = 0; y < frameHeight(); y++)
		for (int x = 0; x < frameWidth(); x++) {
			vfb[y][x] = Color(float(x) / frameWidth(), float(y) / frameHeight(), 0.0f);
		}
}

int main ( int argc, char** argv )
{
	initGraphics(RESX, RESY);
	render();
	displayVFB(vfb);
	waitForUserExit();
	closeGraphics();
	printf("Exited cleanly\n");
	return 0;
}
