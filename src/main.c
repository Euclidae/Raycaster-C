#include <SDL3/SDL_mutex.h>
#include <stdio.h>
#include "globals.h"
#include <stdbool.h>
#include <SDL3/SDL.h>
#include "map.h"

typedef struct Player{
    float x,y, width, height, rotation_angle, turn_speed;
    int turn_direction; //-1 for left. +1 for right
    int walk_direction; //-1 for backwards. +1 for forwards. 0 for stationary
    int walk_speed;

} Player;

Player player;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

float previous_ticks = 0;

bool init(){
  if(!SDL_Init(SDL_INIT_VIDEO)){
    SDL_Log("SDL failed to initialize. ERROR : %s", SDL_GetError());
    return false;
  }
  window = SDL_CreateWindow("Raycaster", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
  if(window == NULL){
    fprintf(stderr,"Failed to generate window. SDL_Error : %s", SDL_GetError());
  }

  renderer = SDL_CreateRenderer(window, NULL);
  return true;
}

void setup(){
    player.x = WINDOW_WIDTH /2;
    player.y = WINDOW_HEIGHT /2;
    player.width = 5.0f;
    player.turn_direction = 0.0f;
    player.walk_direction = 0.0f;
    player.rotation_angle = PI/2;
    player.walk_speed = 100;
    player.turn_speed = 45;
}

bool process_input(){
  SDL_Event event;
  while(SDL_PollEvent(&event)){
    if(event.type == SDL_EVENT_QUIT){
      return false;
    }
    else if(event.type == SDL_EVENT_KEY_DOWN){
      switch(event.key.scancode){
        case SDL_SCANCODE_ESCAPE:
          return false;
          break;

        default:
          break;
      }
    }
  }
  return true;
}

void update(){
    int delay = (int)FRAME_TIME_LENGTH -(int)(SDL_GetTicks() - previous_ticks);
    previous_ticks = (float)SDL_GetTicks();

    if(delay > 0 && delay <= FRAME_TIME_LENGTH){
        SDL_Delay(delay);
    }

    float delta_time = ((float)SDL_GetTicks() - previous_ticks)/1000.0f;
}

void render(){
  SDL_SetRenderDrawColor(renderer,0.3*255,0.9*255,0.7*255,255);
  SDL_RenderClear(renderer);
  render_map(renderer);
  //render_player();
  //render_rays();
  SDL_RenderPresent(renderer);
}

void clean_up(){
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


int main(){
  if(init()){
    while(process_input()){
        render();
        update();
    }
  }
  clean_up();
  return 0;
}
