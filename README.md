# Raycaster in C

A crude raycaster built from scratch that supports:

- Rendering walls based on distance
- Camera (player) rotation 
- Camera (player) movement

I've also included the documentation below, it's only using [SDL2/SDL](https://wiki.libsdl.org/SDL2/FrontPage) so it's very low level, the only requirement to replicate this project is knowledge in basic maths (mostly trig). The build commands can be found after the documentation.

&nbsp;

# Documentation

## 0. Setup

Just to ensure that SDL is properly working, compile a simple window using [SDL_CreateWIndow( )](https://wiki.libsdl.org/SDL2/SDL_CreateWindow)

```c
#include <SDL2/SDL.h>

/* inside main(), first initialize the type of SDL program */
SDL_Init(SDL_INIT_VIDEO);

/* Create a window and a surface to draw on */
SDL_Window *pWindow = SDL_CreateWindow(
	"Le Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 1000, 0
); 

SDL_Surface *pSurface = SDL_GetWindowSurface(pWindow);

/* Make sure the window stays for a while before the program finishes */
SDL_Delay(3000); 
```


Now try to draw a rectangle, using [SDL_UpdateWindowSurface( )](https://wiki.libsdl.org/SDL2/SDL_UpdateWindowSurface) to flush it onto the screen:
```c
//... continue from above
SDL_Rect rect = {200, 200, 100, 100} // x, y, w, h
SDL_FillRect(pSurface, &rect, 0xffffff);
SDL_UpdateWindowSurface(pWindow);
```

| ![screenshot](./screenshos/newscreenshot_059.jpg) |
| --- |
| It took me an hour to figure this out because I'm using Suckless's DT as my terminal emulator, which has trouble rendering stuff on a window, but alas it's done |

Finally, wrap everything inside a while loop with a short delay between each frame:
```c
while (is_running){
	/* rendering method...*/
	SDL_Delay(15);
}
```

And we're ready to go!

&nbsp;

## 1. Configuring the 2D Map

Our end goal is to generate a (fake) 3D map, we can get there by first generating a 2D grid map, then modify it to 3D. 

### Game Assets
---
We don't start with many stuff (the raycaster just needs walls, really), but a few things can be defined right off the bat:

- A global `PLAYER_FOV` value (field of view)
- A `map[][]` 2D grid
- A `Player` struct

```c
#define PLAYER_POV 80

/* 0 = no wall    1 = wall */
int map[5][5] = {
    {0, 0, 0, 1, 0},
    {0, 0, 1, 1, 0},
    {1, 0, 0, 1, 0},
    {1, 0, 0, 0, 0},
    {1, 1, 1, 0, 0},
};

typedef struct{
	float x, y, angle; // what is angle? see illustration
} Player;
```

| ![screenshot](./screenshos/newscreenshot_60.jpg) |
| --- |
| Excalidraw Illustration |

When the player's gaze hits a wall, the distance between the Player and the wall determines the height of the wall, which then gets displayed onto the screen.

So let's work step by step here. First, we need to know when the ray hits a wall.

&nbsp;

## 2. Distance from the nearest wall


The `getDistance(Player player, double angle)` function will return the distance between the player and a wall they're look at from the `angle`. Basically:

```c
double getDistance(Player player, double angle){
	bool wall_detected = false;
	
	double ray_x = player.x;
	double ray_y = player.y;
	
	while(!wall_detected){
		/* ... */
	}
}
```

Inside the while loop, we will check for 3 things:

- ***What is the current distance?*** That is, how far the player's gaze has gone, during the current iteration of the while loop.
```c
/* inside the while loop, calculate the gaze's magnitude */
double ray_distance = sqrt(
	pow(ray_x-player.x, 2) + 
	pow(ray_y-player.y, 2)
);
```

- ***Which cell is the ray currently inside?*** If the cell count exceeds the map size, then the ray has gone outside the map - exit immediately.
```c
/* cell size is a global macro */
int cell_x = ray_x / CELL_SIZE;
int cell_y = ray_y / CELL_SIZE;
```

- ***Have we hit a wall yet?*** If so, return immediately; if not, then grow the ray in the direction of the angle argument.
```c
if(map[cell_y][cell_x] == 1){ // mission accomplished
	wall_detected = true;
	return ray_distance;   
} else {
	/* increment ray_x & ray_y with sine and cosine */
}
```

&nbsp;

## 3. Drawing from the Field of View

Scanning through the field of view, we can start constructing what the player can see, starting from the one of the `FOV` arrows to the other.
![[newscreenshot_062 1.jpg | 400]]
But what's the exact angle at which we start drawing? That angle is essentially `α` - `ϑ`. And we know the values for each angle:

- `ϑ` is `FOV/2`
- `α` is a member of the `Player` struct

The result would be a `for` loop:
```c
for (double angle = player.angle - PLAYER_FOV/2;
	angle <= player.angle + PLAYER_FOV/2; angle += 0.1){
	
	/* 1. Get distance of the closest wall to the player at this angle
	** 2. Draw the vertical line based on distance
	 */	
}
```

### Inside the `for` loop
---
For every possible angle within the designated field of view, we will:
- Shoot a gaze from the player's eye
- Get the distance between that gaze and the wall that it hits
- Draw a vertical line based on the distance

| ![screenshot](./screenshos/newscreenshot_63.jpg) |
| --- |
| Excalidraw Illustration |

A separate `drawVerticalLine()` function is required, but all it does is feeding the correct arguments to SDL's inherent `SDL_FillRect()` function that draws a `SDL_Rect` object. Remember, a line is just a thin rectangle. 

The difficult part is figuring out the correct arguments (or, properties) of the rectangle. Recall that a `SDL_Rect` struct:
```c
typedef struct SDL_Rect
{
    int x, y; // x and y coordinates. Top left corner of the screen is (0,0)
    int w, h; // width and height
} SDL_Rect;
```

##### The `x` property
The `x` position of our `SDL_Rect` is simply a modified `angle` value of the current `for` loop iteration, and it makes sense: the smaller the angle, the more-to-the-left the `SDL_Rect` will be, since we're scanning from left to right. It requires a bit of thinking to obtain the modification values to adjust the original `angle`:

-  `angle_offset` is needed to remove the part outside `PLAYER_FOV`
-  `angle_scale` is needed because the entire `for` loop only runs for the duration of the `PLAYER_FOV` angle (in degrees). So let's say if `PLAYER_FOV` is only 80 degrees but the screen is `900` pixels wide, the `for` loop will just stop at the 80th pixel.

```c
double angle_offset = player.angle - PLAYER_FOV/2;  // player.angle is the fixed centre of the player's vision
double angle_scale = SCREEN_WIDTH / PLAYER_FOV;

double x = (angle - angle_offset) * angle_scale;
```

##### The `y` property
The `y` position of the `SDL_Rect` is much easier to get if you consult the graph I drew:
![[newscreenshot_065.jpg | 500]]
```c
double y = SCREEN_HEIGHT/2 - rectangle_height; 
```

But what's `rectangle_height`? Well...

##### The `h` property
The height of the rectangle, denoted as `visual_height` in the program, is directly inproportional to the `distance` value that gets calculated per iteration, and is the core of this entire raycaster.

```c
double height_scale = 50;
double visual_height = SCREEN_HEIGHT / distance * height_scale;
```

Again, it should be intuitive: the further a wall is (and, the larger the `distance`), the smaller the height of the wall that gets displayed on the screen will be. For example, if the player sees a wall at the end of the map (`600` pixels away), then the wall gets rendered as `600/600=1` pixel tall, which makes sense, because it's the wall furthest distance away.

However, we still want to make it visible even it's far away - hence the scaler multiple.

##### The `w` property
This width of the rectangle is just `1` or `2` pixel. Doesn't really matter. 

And we're done! The polishing steps would be to add player movement & camera rotation, but those are trivial tasks. The program is basically completed at this point.

&nbsp;

# Build

This raycaster is built for Linux. First make sure SDL2 is installed on your machine, then run the build command:

```shell
gcc -Wall -o raycaster raycaster.c -lm `sdl2-config --cflags --libs`
```

Calling the executable:

```shell
./raycaster
```
