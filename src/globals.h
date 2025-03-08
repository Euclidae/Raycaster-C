#ifndef GLOBALS_H_
#define GLOBALS_H_
#define PI 3.14159264
#define TWO_PI 6.28318530

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define FOV_ANGLE (60 * PI/180) //In radians
//
#define NUM_RAYS WINDOW_WIDTH // 1 Ray per pixel column

#define FPS 30
#define FRAME_TIME_LENGTH 1000.0/FPS //How many milliseconds per frame.
#define MINI_MAP_SCALE_FACTOR 1.0f

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#endif
