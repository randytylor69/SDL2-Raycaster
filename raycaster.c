/* build command:
 * gcc -o raycaster raycaster.c `sdl2-config --cflags --libs`
 */

#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xffffff

bool is_running = true;

int main(){
    /* initialize SDL objects */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *pWindow = SDL_CreateWindow("Le Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Surface *pSurface = SDL_GetWindowSurface(pWindow);
    
    int count = 0;

    while (is_running){
	/* drawing */
	SDL_Rect rect = {SCREEN_WIDTH/2 + count, SCREEN_HEIGHT/2 + count, 100, 100};
	SDL_FillRect(pSurface, &rect, COLOR_WHITE);
	SDL_UpdateWindowSurface(pWindow);
	count += 10;
	/* finishing */
	SDL_Delay(15);
    }
    return 0;
}
