#include "space.h"
#include "pebble.h"

static Window *window;
static Layer *space_layer, *planet_layer;

static void space_update_proc(Layer *layer, GContext *ctx) {
  #ifdef PBL_BW
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  #elif PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorOxfordBlue);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  #endif
}

static void planet_update_proc(Layer *layer, GContext *ctx) {  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  #ifdef PBL_RECT
    int16_t earth_dist      = 39;
    int16_t moon_dist       = 21;
    int16_t asteroid_dist   = 10;
  
    int16_t sun_radius      = 10;
    int16_t earth_radius    = 6;
    int16_t moon_radius     = 4;
    int16_t asteroid_radius = 1;

    int32_t e               = 1.25;
  #elif PBL_ROUND
    int16_t earth_dist      = 46;
    int16_t moon_dist       = 24;
    int16_t asteroid_dist   = 11;
  
    int16_t sun_radius      = 11;
    int16_t earth_radius    = 7;
    int16_t moon_radius     = 4;
    int16_t asteroid_radius = 1;

    int32_t e               = 1.0;
  #endif
  
  int32_t asteroid_angle  = TRIG_MAX_ANGLE * t->tm_sec / 60;
  int32_t moon_angle      = TRIG_MAX_ANGLE * t->tm_min / 60;
  int32_t earth_angle     = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  // sun
  #ifdef PBL_BW
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, center, sun_radius);
  #elif PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorYellow);
    graphics_fill_circle(ctx, center, sun_radius);
  #endif
  
  // earth
  GPoint earth = {
    .x = (int16_t)(sin_lookup(earth_angle) * (int32_t)earth_dist / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(earth_angle) * (int32_t)earth_dist * (int32_t)e / TRIG_MAX_RATIO) + center.y,
  };
  
  #ifdef PBL_BW
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, earth, earth_radius);
  #elif PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorMidnightGreen);
    graphics_fill_circle(ctx, earth, earth_radius);
  #endif 
  
   // moon
  GPoint moon = {
    .x = (int16_t)(sin_lookup(moon_angle) * (int32_t)moon_dist / TRIG_MAX_RATIO) + earth.x,
    .y = (int16_t)(-cos_lookup(moon_angle) * (int32_t)moon_dist / TRIG_MAX_RATIO) + earth.y,
  };
  
  #ifdef PBL_BW
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, moon, moon_radius);
  #elif PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorBlueMoon);
    graphics_fill_circle(ctx, moon, moon_radius);
  #endif

    // asteroid
  GPoint asteroid = {
    .x = (int16_t)(sin_lookup(asteroid_angle) * (int32_t)asteroid_dist / TRIG_MAX_RATIO) + moon.x,
    .y = (int16_t)(-cos_lookup(asteroid_angle) * (int32_t)asteroid_dist / TRIG_MAX_RATIO) + moon.y,
  };

  #ifdef PBL_BW
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, asteroid, asteroid_radius);
  #elif PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorPastelYellow);
    graphics_fill_circle(ctx, asteroid, asteroid_radius);
  #endif
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  space_layer = layer_create(bounds);
  layer_set_update_proc(space_layer, space_update_proc);
  layer_add_child(window_layer, space_layer);

  planet_layer = layer_create(bounds);
  layer_set_update_proc(planet_layer, planet_update_proc);
  layer_add_child(window_layer, planet_layer);
}

static void window_unload(Window *window) {
  layer_destroy(space_layer);
  layer_destroy(planet_layer);
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}