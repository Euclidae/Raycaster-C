#include <SDL3/SDL_render.h>
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
    bool was_hit_vertical;
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

float normalizeAngle(float angle) {
    angle = remainder(angle, TWO_PI);
    if (angle < 0) {
        angle = TWO_PI + angle;
    }
    return angle;
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void cast_ray(float ray_angle, int stripId) {
    ray_angle = normalizeAngle(ray_angle);

    bool isRayFacingDown = ray_angle > 0 && ray_angle < PI;
    bool isRayFacingUp = !isRayFacingDown;

    bool isRayFacingRight = ray_angle < 0.5 * PI || ray_angle > 1.5 * PI;
    bool isRayFacingLeft = !isRayFacingRight;

    float xintercept, yintercept;
    float xstep, ystep;

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool found_horz_wall_hit = false;
    float horz_wall_hit_X = 0;
    float horz_wall_hit_Y = 0;
    float horz_wall_content = 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tan(ray_angle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp ? -1 : 1;

    xstep = TILE_SIZE / tan(ray_angle);
    xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);

        if (map_has_wall_at(xToCheck, yToCheck)) {
            // found a wall hit
            horz_wall_hit_X = nextHorzTouchX;
            horz_wall_hit_Y = nextHorzTouchY;
            horz_wall_content = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            found_horz_wall_hit = true;
            break;
        } else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool found_vert_wall_hit = false;
    float vert_wall_hit_X = 0;
    float vert_wall_hit_Y = 0;
    float vert_wall_content = 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tan(ray_angle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft ? -1 : 1;

    ystep = TILE_SIZE * tan(ray_angle);
    ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
        float yToCheck = nextVertTouchY;

        if (map_has_wall_at(xToCheck, yToCheck)) {
            // found a wall hit
            vert_wall_hit_X = nextVertTouchX;
            vert_wall_hit_Y = nextVertTouchY;
            vert_wall_content = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            found_vert_wall_hit = true;
            break;
        } else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = found_horz_wall_hit
        ? distanceBetweenPoints(player.x, player.y, horz_wall_hit_X, horz_wall_hit_Y)
        : INT_MAX;
    float vertHitDistance = found_vert_wall_hit
    ? distanceBetweenPoints(player.x, player.y, vert_wall_hit_X, vert_wall_hit_Y)
        : INT_MAX;

    if (vertHitDistance < horzHitDistance) {
        rays[stripId].distance = vertHitDistance;
        rays[stripId].wall_hit_x = vert_wall_hit_X;
        rays[stripId].wall_hit_y = vert_wall_hit_Y;
        rays[stripId].wall_hit_content = vert_wall_content;
        rays[stripId].was_hit_vertical = true;
    } else {
        rays[stripId].distance = horzHitDistance;
        rays[stripId].wall_hit_x = horz_wall_hit_X;
        rays[stripId].wall_hit_y = horz_wall_hit_Y;
        rays[stripId].wall_hit_content = horz_wall_content;
        rays[stripId].was_hit_vertical = false;
    }
    rays[stripId].ray_angle = ray_angle;
    rays[stripId].is_ray_facing_down = isRayFacingDown;
    rays[stripId].is_ray_facing_up = isRayFacingUp;
    rays[stripId].is_ray_facing_left = isRayFacingLeft;
    rays[stripId].is_ray_facing_right = isRayFacingRight;
}

void cast_all_rays() {
    // Start at the leftmost angle of the FOV
    float ray_angle = player.rotation_angle - (FOV_ANGLE / 2);

    for(int strip_id = 0; strip_id < NUM_RAYS; strip_id++) {
        cast_ray(ray_angle, strip_id);
        // Move to the next angle within the FOV
        ray_angle += FOV_ANGLE / NUM_RAYS;
    }
}

void render_rays(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for(int i = 0; i < NUM_RAYS; ++i){
        SDL_RenderLine(renderer,player.x*MINI_MAP_SCALE_FACTOR,player.y*MINI_MAP_SCALE_FACTOR,rays[i].wall_hit_x*MINI_MAP_SCALE_FACTOR,rays[i].wall_hit_y*MINI_MAP_SCALE_FACTOR);
    }
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
    cast_all_rays();
}

void render(){
  SDL_SetRenderDrawColor(renderer,0.3*255,0.9*255,0.7*255,255);
  SDL_RenderClear(renderer);
  render_map(renderer);
  render_player();
  render_rays();
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
