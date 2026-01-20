#include <pebble.h>

static Window* s_main_window;
static TextLayer* s_time_layer;
static GFont s_time_font;
static BitmapLayer* s_kitty_layer;
static GBitmap* s_kitty_bitmap;
static int s_battery_level;
static TextLayer* s_date_layer;
static TextLayer* s_battery_layer;

static void update_time() {
  // get a tm (time) structure
  time_t temp = time(NULL);
  struct tm* tick_time = localtime(&temp);

  // write current time (hrs&mins) into a buffer
  static char s_buffer[8];
  static char date_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(date_buffer, sizeof(date_buffer), "%d", tick_time);

  // set textlayer text to string buffer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
  update_time();
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;

  static char s_buffer[8];
  snprintf(s_buffer, sizeof(s_buffer), "%d%%", s_battery_level);

  text_layer_set_text(s_battery_layer, s_buffer);
}

static void main_window_load(Window* window) {
  // get window info
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // create gbitmap
  s_kitty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_KITTY);

  // create bitmap layer
  s_kitty_layer = bitmap_layer_create(bounds);

  // set the kitty onto the layer, then add to window
  bitmap_layer_set_bitmap(s_kitty_layer, s_kitty_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_kitty_layer));
  bitmap_layer_set_alignment(s_kitty_layer, GAlignBottom);

  // create gfont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

  // create time textlayer
  s_time_layer = text_layer_create(GRect(0, 0, bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // create date textlayer
  s_date_layer = text_layer_create(GRect(0, 50, bounds.size.w, 50));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);

  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // create battery layer
  s_battery_layer = text_layer_create(GRect(0, 50, bounds.size.w, 50));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));

  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
}

static void main_window_unload(Window* window) {
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  bitmap_layer_destroy(s_kitty_layer);
  gbitmap_destroy(s_kitty_bitmap);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
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

  // register battery service
  battery_state_service_subscribe(battery_callback);

  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}