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

typedef struct Ray{
    float ray_angle;
    float wall_hit_x;
    float wall_hit_y;
    float distance;
    bool was_his_vertical;
    bool is_ray_facing_up;
    bool is_ray_facing_down;
    bool is_ray_facing_left;
    bool is_ray_facing_right;
    int wall_hit_content;
} Ray;

Ray rays[NUM_RAYS];

Player player = {
    .x = WINDOW_WIDTH /2.0f,
    .y = WINDOW_HEIGHT /2.0f,
    .width = 15.0f,
    .height = 15.0f,
    .turn_direction = 0,
    .walk_direction = 0,
    .rotation_angle = PI/2,
    .turn_speed = 45,
    .walk_speed = 100,
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

float normalize_angle(float angle){
    angle = remainder(angle, TWO_PI);
    if(angle < 0){
        angle = TWO_PI + angle;
    }
    return angle;
}

void cast_ray(float ray_angle,int strip_id){
    ray_angle = normalize_angle(ray_angle);

    bool is_ray_facing_up = ray_angle > 0 && ray_angle < PI;
    bool is_ray_facing_down = !is_ray_facing_up;
    bool is_ray_facing_left = ray_angle < (0.5*PI) || ray_angle > (1.5 * PI);
    bool is_ray_facing_right =  !is_ray_facing_left;

    float x_step, y_step;
    float x_intercept, y_intercept;

    bool found_horz_wall_hit = false;
    float horz_wall_hit_x = 0;
    float horz_wall_hit_y = 0;
    int horz_wall_content = 0;

    y_intercept = floor(player.y /TILE_SIZE) * TILE_SIZE;
    y_intercept += is_ray_facing_down? TILE_SIZE : 0;

    x_intercept = player.x + (y_intercept - player.y) /tan(ray_angle);

    y_step = TILE_SIZE;
    y_step *= is_ray_facing_up? -1 : 1;

    x_step = TILE_SIZE / tan(ray_angle);
    x_step *= (is_ray_facing_left && x_step > 0)? -1 : 1;
    x_step *= (is_ray_facing_right && x_step < 0)? -1 : 1;

    float next_horz_touch_x = x_intercept;
    float next_horz_touch_y = y_intercept;

    while(next_horz_touch_x >= 0 && next_horz_touch_x <= WINDOW_WIDTH && next_horz_touch_y >= 0 && next_horz_touch_y <= WINDOW_HEIGHT){
        float x_to_check = next_horz_touch_x;
        float y_to_check = next_horz_touch_y + (is_ray_facing_up? -1 : 0);

        if(map_has_wall_at(x_to_check, y_to_check)){
            //found the wall ollision
            horz_wall_hit_x = next_horz_touch_x;
            horz_wall_hit_y = next_horz_touch_y;
            found_horz_wall_hit = true;
            horz_wall_content = map[(int)floor(y_to_check / TILE_SIZE)][(int)floor(x_to_check/TILE_SIZE)];
        }else{
            next_horz_touch_x += x_step;
            next_horz_touch_y += y_step;
        }
    }

}

void cast_all_rays(){
    float ray_angle = player.rotation_angle - FOV_ANGLE/2;
    for(int strip_id = 0;  strip_id < NUM_RAYS; ++strip_id){
        cast_ray(ray_angle,strip_id);
        ray_angle+=FOV_ANGLE/NUM_RAYS;
    }
}
void render_rays(){

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
    (player.x * MINI_MAP_SCALE_FACTOR) + cos(player.rotation_angle) * 20 ,
    (player.y * MINI_MAP_SCALE_FACTOR) + sin(player.rotation_angle) * 20
    );
    SDL_RenderFillRect(renderer,&player_rect);
}

void move_player(float delta_time){
    //turn direction and turn speed create the angle. delta time just attaches that angle to time
    player.rotation_angle += player.turn_direction * delta_time * player.turn_speed;
    float move_step = player.walk_direction * player.walk_speed * delta_time;

    if(map_has_wall_at(player.x, player.y)){
        player.x += -1;
        player.y += -1;
    }else{
        player.x += (cos(player.rotation_angle) * move_step);
        player.y += (sin(player.rotation_angle) * move_step);
    }
}

void update(){
    float current_ticks = (float)SDL_GetTicks()/1000.0f;
    float delta_time = current_ticks - previous_ticks;
    previous_ticks = current_ticks;

    // Cap frame rate if necessary
    int delay = (int)(FRAME_TIME_LENGTH - (SDL_GetTicks() - previous_ticks));
    // if(delay > 0 && delay <= FRAME_TIME_LENGTH){
    //     SDL_Delay(delay);
    // }
    SDL_Delay((int)FRAME_TIME_LENGTH);
    move_player(delta_time);
    //cast_all_rays();
}

void render(){
  SDL_SetRenderDrawColor(renderer,0.3*255,0.9*255,0.7*255,255);
  SDL_RenderClear(renderer);
  render_map(renderer);
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
    while(process_input()){
        update();
        render();
    }
  }
  clean_up();
  return 0;
}
