#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <stdio.h>
#include <math.h>
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

Player player = {
    .x = WINDOW_WIDTH /2.0f,
    .y = WINDOW_HEIGHT /2.0f,
    .width = 15.0f,
    .height = 15.0f,
    .turn_direction = 0,
    .walk_direction = 0,
    .rotation_angle = PI/2,
    .walk_speed = 100,
    .turn_speed = 45
};

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
        case SDL_SCANCODE_UP:
            player.walk_direction = 1;
            break;

        case SDL_SCANCODE_DOWN:
             player.walk_direction = -1;
             break;

        case SDL_SCANCODE_LEFT:
             player.turn_direction = -1;
             break;

        case SDL_SCANCODE_RIGHT:
            player.turn_direction = 1;
            break;

        default:
          break;
      }
    }else if(event.type == SDL_EVENT_KEY_UP){
        switch(event.key.scancode){
          case SDL_SCANCODE_UP:
              player.walk_direction = 0;
              break;

          case SDL_SCANCODE_DOWN:
               player.walk_direction = 0;
               break;

          case SDL_SCANCODE_LEFT:
               player.turn_direction = 0;
               break;

          case SDL_SCANCODE_RIGHT:
              player.turn_direction = 0;
              break;

          default:
            break;
    }
    }
  }
  return true;
}

void render_player(){
    SDL_SetRenderDrawColor(renderer, 255,255,255,255);
    SDL_FRect player_rect = {
        (float)player.x * MINI_MAP_SCALE_FACTOR,
        (float)player.y * MINI_MAP_SCALE_FACTOR,
        (float)player.width * MINI_MAP_SCALE_FACTOR,
        (float)player.height * MINI_MAP_SCALE_FACTOR
    };

    SDL_RenderLine(
    renderer,
    player.x * MINI_MAP_SCALE_FACTOR,
    player.y * MINI_MAP_SCALE_FACTOR,
    (player.x * MINI_MAP_SCALE_FACTOR) + cos(player.rotation_angle) * 40 ,
    (player.y * MINI_MAP_SCALE_FACTOR) + sin(player.rotation_angle) * 40
    );
    SDL_RenderFillRect(renderer,&player_rect);
}

void move_player(float delta_time){
    player.rotation_angle += player.turn_direction * delta_time*player.turn_speed;
    float move_step = player.walk_direction * player.walk_speed * delta_time;

    float new_player_x = player.x + cos(player.rotation_angle) * move_step;
    float new_player_y = player.y + sin(player.rotation_angle) * move_step;

    player.x = new_player_x;
    player.y = new_player_y;
}

void update(){
    int delay = (int)FRAME_TIME_LENGTH -(int)(SDL_GetTicks() - previous_ticks);
    previous_ticks = (float)SDL_GetTicks();

    if(delay > 0 && delay <= FRAME_TIME_LENGTH){
        SDL_Delay(delay);
    }

    float delta_time = ((float)SDL_GetTicks() - previous_ticks)/1000.0f;
    move_player(delta_time);
}

void render(){
  SDL_SetRenderDrawColor(renderer,0.3*255,0.9*255,0.7*255,255);
  SDL_RenderClear(renderer);
  //render_map(renderer);
  render_player();
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
    setup();
    while(process_input()){
        update();
        render();
    }
  }
  clean_up();
  return 0;
}
