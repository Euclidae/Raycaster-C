#include "map.h"
#include "globals.h"
#include <SDL3/SDL_render.h>

void render_map(SDL_Renderer* renderer){

    for(int i = 0; i < MAP_NUM_COLS; ++i){
        for(int j = 0; j < MAP_NUM_ROWS; ++j){
            SDL_Color tile_color;
            float tile_x, tile_y;

            tile_x = (float)TILE_SIZE * i;
            tile_y = (float)TILE_SIZE * j;

            if(map[i][j] == 0){
                tile_color.r = 0;
                tile_color.g = 0;
                tile_color.b = 0;
                tile_color.a = 255;
            }else{
                tile_color.r = 255;
                tile_color.g = 255;
                tile_color.b = 255;
                tile_color.a = 255;
            }

            SDL_FRect tile = {
                .x = tile_x * MINI_MAP_SCALE_FACTOR,
                .y = tile_y * MINI_MAP_SCALE_FACTOR,
                .w = TILE_SIZE * MINI_MAP_SCALE_FACTOR,
                .h = TILE_SIZE * MINI_MAP_SCALE_FACTOR
            };
            SDL_SetRenderDrawColor(renderer, tile_color.r, tile_color.g, tile_color.b, tile_color.a);
            SDL_RenderFillRect(renderer,&tile);
        }
    }
}
