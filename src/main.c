#include <SDL3/SDL_render.h>
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

float normalize_angle(float angle){
    angle = remainder(angle, TWO_PI);
    if(angle < 0){
        angle = TWO_PI + angle;
    }
    return angle;
}

float distance_between_points(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void cast_ray(float ray_angle, int strip_id) {
    // Normalize the angle to ensure it's between 0 and 2*PI
    ray_angle = normalize_angle(ray_angle);

    // Determine ray direction flags
    bool is_ray_facing_up = ray_angle > 0 && ray_angle < PI;
    bool is_ray_facing_down = !is_ray_facing_up;
    bool is_ray_facing_left = ray_angle < (0.5 * PI) || ray_angle > (1.5 * PI);
    bool is_ray_facing_right = !is_ray_facing_left;

    float x_step, y_step;
    float x_intercept, y_intercept;

    /**************** HORIZONTAL RAY-GRID INTERSECTION CODE ****************/
    bool found_horz_wall_hit = false;
    float horz_wall_hit_x = 0;
    float horz_wall_hit_y = 0;
    int horz_wall_content = 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    y_intercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    y_intercept += is_ray_facing_down ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    if (fabs(sin(ray_angle)) < 0.000001) {
        // Handle nearly horizontal rays to avoid division by zero
        x_intercept = player.x;
    } else {
        x_intercept = player.x + (y_intercept - player.y) / tan(ray_angle);
    }

    // Calculate the increment xstep and ystep
    y_step = TILE_SIZE;
    y_step *= is_ray_facing_up ? -1 : 1;

    if (fabs(sin(ray_angle)) < 0.000001) {
        // If ray is nearly horizontal, make x_step very large in the correct direction
        x_step = is_ray_facing_right ? WINDOW_WIDTH : -WINDOW_WIDTH;
    } else {
        x_step = TILE_SIZE / tan(ray_angle);
        // Ensure x_step is in the correct direction
        x_step *= (is_ray_facing_left && x_step > 0) ? -1 : 1;
        x_step *= (is_ray_facing_right && x_step < 0) ? -1 : 1;
    }

    float next_horz_touch_x = x_intercept;
    float next_horz_touch_y = y_intercept;

    // Increment xstep and ystep until we find a wall
    int max_checks = 1000; // Prevent infinite loops
    int checks = 0;

    while (next_horz_touch_x >= 0 && next_horz_touch_x <= WINDOW_WIDTH &&
           next_horz_touch_y >= 0 && next_horz_touch_y <= WINDOW_HEIGHT &&
           checks < max_checks) {

        float x_to_check = next_horz_touch_x;
        float y_to_check = next_horz_touch_y + (is_ray_facing_up ? -1 : 0);

        if (map_has_wall_at(x_to_check, y_to_check)) {
            // Found a wall hit
            horz_wall_hit_x = next_horz_touch_x;
            horz_wall_hit_y = next_horz_touch_y;
            found_horz_wall_hit = true;
            horz_wall_content = map[(int)floor(y_to_check / TILE_SIZE)][(int)floor(x_to_check / TILE_SIZE)];
            break;
        } else {
            next_horz_touch_x += x_step;
            next_horz_touch_y += y_step;
            checks++;
        }
    }

    /**************** VERTICAL RAY-GRID INTERSECTION CODE ****************/
    bool found_vert_wall_hit = false;
    float vert_wall_hit_x = 0;
    float vert_wall_hit_y = 0;
    int vert_wall_content = 0;

    // Find the x-coordinate of the closest vertical grid intersection
    x_intercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    x_intercept += is_ray_facing_right ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest vertical grid intersection
    if (fabs(cos(ray_angle)) < 0.000001) {
        // Handle nearly vertical rays to avoid division by zero
        y_intercept = player.y;
    } else {
        y_intercept = player.y + (x_intercept - player.x) * tan(ray_angle);
    }

    // Calculate the increment xstep and ystep
    x_step = TILE_SIZE;
    x_step *= is_ray_facing_left ? -1 : 1;

    if (fabs(cos(ray_angle)) < 0.000001) {
        // If ray is nearly vertical, make y_step very large in the correct direction
        y_step = is_ray_facing_down ? WINDOW_HEIGHT : -WINDOW_HEIGHT;
    } else {
        y_step = TILE_SIZE * tan(ray_angle);
        // Ensure y_step is in the correct direction
        y_step *= (is_ray_facing_up && y_step > 0) ? -1 : 1;
        y_step *= (is_ray_facing_down && y_step < 0) ? -1 : 1;
    }

    float next_vert_touch_x = x_intercept;
    float next_vert_touch_y = y_intercept;

    // Increment xstep and ystep until we find a wall
    checks = 0;

    while (next_vert_touch_x >= 0 && next_vert_touch_x <= WINDOW_WIDTH &&
           next_vert_touch_y >= 0 && next_vert_touch_y <= WINDOW_HEIGHT &&
           checks < max_checks) {

        float x_to_check = next_vert_touch_x + (is_ray_facing_left ? -1 : 0);
        float y_to_check = next_vert_touch_y;

        if (map_has_wall_at(x_to_check, y_to_check)) {
            // Found a wall hit
            vert_wall_hit_x = next_vert_touch_x;
            vert_wall_hit_y = next_vert_touch_y;
            found_vert_wall_hit = true;
            vert_wall_content = map[(int)floor(y_to_check / TILE_SIZE)][(int)floor(x_to_check / TILE_SIZE)];
            break;
        } else {
            next_vert_touch_x += x_step;
            next_vert_touch_y += y_step;
            checks++;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horz_hit_distance = found_horz_wall_hit ?
        distance_between_points(player.x, player.y, horz_wall_hit_x, horz_wall_hit_y) :
        INT_MAX;  // Use FLT_MAX instead of INT_MAX for float comparison

    float vert_hit_distance = found_vert_wall_hit ?
        distance_between_points(player.x, player.y, vert_wall_hit_x, vert_wall_hit_y) :
        INT_MAX;  // Use FLT_MAX instead of INT_MAX for float comparison

    // Store the smallest of the distances
    if (vert_hit_distance < horz_hit_distance) {
        rays[strip_id].distance = vert_hit_distance;
        rays[strip_id].wall_hit_x = vert_wall_hit_x;
        rays[strip_id].wall_hit_y = vert_wall_hit_y;
        rays[strip_id].wall_hit_content = vert_wall_content;
        rays[strip_id].was_hit_vertical = true;
    } else {
        rays[strip_id].distance = horz_hit_distance;
        rays[strip_id].wall_hit_x = horz_wall_hit_x;
        rays[strip_id].wall_hit_y = horz_wall_hit_y;
        rays[strip_id].wall_hit_content = horz_wall_content;
        rays[strip_id].was_hit_vertical = false;
    }

    // Save the ray angle and direction flags
    rays[strip_id].ray_angle = ray_angle;
    rays[strip_id].is_ray_facing_down = is_ray_facing_down;
    rays[strip_id].is_ray_facing_left = is_ray_facing_left;
    rays[strip_id].is_ray_facing_right = is_ray_facing_right;
    rays[strip_id].is_ray_facing_up = is_ray_facing_up;
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
        SDL_RenderLine(renderer,player.x,player.y,rays[i].wall_hit_x,rays[i].wall_hit_y);
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
