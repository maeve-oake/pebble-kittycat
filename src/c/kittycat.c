#include <pebble.h>

static Window* s_main_window;
static TextLayer* s_time_layer;

static void update_time() {
  // get a tm (time) structure
  time_t temp = time(NULL);
  struct tm* tick_time = localtime(&temp);

  // write current time (hrs&mins) into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // set textlayer text to string buffer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window* window) {
  // get window info
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // create textlayer
  s_time_layer = text_layer_create(
    GRect(0, 58, bounds.size.w, 50)
  );

  // textlayer elements
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // add textlayer to Window layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window* window) {
  text_layer_destroy(s_time_layer);
}

static void init() {
  // create window element
  s_main_window = window_create();

  // set handlers to manage window elements
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
      .unload = main_window_unload
  });

  // display window on watch
  window_stack_push(s_main_window, true);

  // register TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // time is displayed correctly at init
  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}