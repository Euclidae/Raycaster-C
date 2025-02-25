#include <stdio.h>
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

}

void render(){
  SDL_SetRenderDrawColor(renderer,0.3*255,0.7*255,0.7*255,255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
}

void clean_up(){
  SDL_Quit();
}


int main(){
  init();
  while(process_input()){
    update();
    render();
  }
  clean_up();
  return 0;
}


