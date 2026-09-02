/* build command:
 *  clear && gcc -Wall -o raycaster raycaster.c -lm `sdl2-config --cflags --libs` && ./raycaster
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xffffff
#define COLOR_GREEN 0x32a852
#define COLOR_BLACK 0x000000
#define CELL_SIZE 50

#define MAP_WIDTH 5
#define MAP_HEIGHT 5
#define SCENE_WIDTH MAP_WIDTH*CELL_SIZE
#define SCENE_HEIGHT MAP_HEIGHT*CELL_SIZE
#define PLAYER_FOV 80  

/* getDistance macros */
#define RAY_SIZE_MODIFIER 1
#define RENDER_DISTANCE 250

/* calculation macros*/
#define DEG2RAD 3.141592653589793/180.0

bool is_running = true;

/* 0 = no wall    1 = wall */
int map[MAP_HEIGHT][MAP_WIDTH] = {
    {0, 0, 0, 1, 0},
    {0, 0, 1, 1, 0},
    {1, 0, 0, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 0, 1},
};

typedef struct{
    double x, y, angle; // angle between the direction that the player's looking and the top window border
} Player;

void listenToKeyboardInput(SDL_Event event){
    while (SDL_PollEvent(&event)){
	switch(event.type){
	    case SDL_KEYDOWN:
		switch(event.key.keysym.sym){
		    /* Quit the program */
		    case SDLK_q:
			is_running = false;
			break;
		}
	}
    }
}
void drawPlayer(SDL_Surface *pSurface, Player player){
    SDL_Rect player_rect = {player.x, player.y, 10, 10};
    SDL_FillRect(pSurface, &player_rect, COLOR_GREEN);
}

/* @RETURN distance to the next wall 
 * assume the player looks in given direction */
double getDistance(Player player, double angle){
    bool wall_detected = false;
    double ray_x = player.x;
    double ray_y = player.y;

    while (!wall_detected){
	
	/* ray_distance is magnitude of the player's gaze so far */
	double ray_distance = sqrt(pow(ray_x-player.x, 2) + pow(ray_y-player.y, 2));
	if (ray_distance > RENDER_DISTANCE){
	    return -1;
	}

	/* which cell is the ray inside ? */
	int cell_x = ray_x / CELL_SIZE;
	int cell_y = ray_y / CELL_SIZE;
	printf("cell values: %d, %d\n", cell_x, cell_y);

	if (cell_x >= MAP_WIDTH || cell_y >= MAP_HEIGHT || cell_x < 0 || cell_y < 0){
	    return -1;
	}

	/* has it hit a wall? */
	if (map[cell_y][cell_x]==1){
	    wall_detected = true;
	    return ray_distance;
	} else {
	    /* move ray along the angle that it's shot */
	    ray_x += cos(angle)*RAY_SIZE_MODIFIER;
	    ray_y += sin(angle)*RAY_SIZE_MODIFIER;
	}

    }
    return -1;
}

/* draw vertical line centered around the horizontal centre axis */
void drawVerticalLine(SDL_Surface *pSurface, double height, double x_pos, Uint32 color){
    SDL_Rect vertical_rect = {x_pos, SCREEN_HEIGHT/2 - height/2, 1, height};
    SDL_FillRect(pSurface, &vertical_rect, color);
}

void drawFOV(SDL_Surface *pSurface, Player player){

    double angle_scale = SCREEN_WIDTH / PLAYER_FOV;
    double angle_offset = player.angle - PLAYER_FOV/2;

    for (double angle = player.angle-PLAYER_FOV/2; 
	angle <= (player.angle + PLAYER_FOV/2); angle += 0.1){

	/* check distance for every angle (radians) in field of view.
	 * but note that PLAYER_POV & player.angle are in degrees */
	double distance = getDistance(player, angle*DEG2RAD);

	/* based on distance, draw vertical line */
	double visual_height = SCREEN_HEIGHT * 50 /distance;
	double vertical_line_x_pos = (angle-angle_offset)*angle_scale;

	drawVerticalLine(pSurface, visual_height, vertical_line_x_pos, COLOR_WHITE);
    }
}

int main(){

    /* initialize SDL objects */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *pWindow = SDL_CreateWindow(
	    "Le Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Surface *pSurface = SDL_GetWindowSurface(pWindow);
    /* initialize game assets */
    Player player = {0,0,45};

    while (is_running){
	/* listening to keyboard input */
	SDL_Event event;
	listenToKeyboardInput(event);

	/* drawing */
	SDL_FillRect(pSurface, NULL, COLOR_BLACK); // force flush whole screen
	drawFOV(pSurface, player);
	SDL_UpdateWindowSurface(pWindow);
		
	/* update */
	player.x += 0.1;
	player.y += 0.1;
	/* finishing */
	SDL_Delay(15);
    }
    return 0;
}
