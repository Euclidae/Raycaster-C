#include <SDL3/SDL_mutex.h>
#include <stdio.h>
#include "globals.h"
#include <stdbool.h>
#include <SDL3/SDL.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

bool init(){
  if(!SDL_Init(SDL_INIT_VIDEO)){
    SDL_Log("SDL failed to initialize. ERROR : %s", SDL_GetError());
    return false;
  }
  window = SDL_CreateWindow("Raycaster", 1920, 1080, SDL_WINDOW_RESIZABLE);
  if(window == NULL){
    fprintf(stderr,"Failed to generate window. SDL_Error : %s", SDL_GetError());
  }

  renderer = SDL_CreateRenderer(window, NULL);
  return true;
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
