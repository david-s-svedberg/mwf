#include <pebble.h>
#include "setup.h"
#include "update.h"
#include "settings.h"

static Window *main_window;

static TextLayer *time_layer;
static TextLayer *week_numer_layer;
static TextLayer *week_day_layer;
static TextLayer *date_layer;
static Layer *battery_layer;

static void setup_battery_layer(Layer *window_layer, GRect bounds) {
  battery_layer = layer_create(GRect(55, -5, bounds.size.w - 55, 30));
  GRect batteryLayerBounds = layer_get_bounds(battery_layer);

  init_battery_indicator(batteryLayerBounds);
  
  layer_set_update_proc(battery_layer, update_battery_layer);
  layer_add_child(window_layer, battery_layer);
}

static void setup_time_layer(Layer *window_layer, GRect bounds) {
  time_layer = text_layer_create(GRect(0, 50, bounds.size.w, 45));
  
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
}

static void setup_date_layer(Layer *window_layer, GRect bounds) {
  date_layer = text_layer_create(GRect(0, 135, bounds.size.w, 30));
  
  text_layer_set_background_color(date_layer, GColorBlack);
  text_layer_set_text_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

static void setup_week_day_layer(Layer *window_layer, GRect bounds) {
  week_day_layer = text_layer_create(GRect(0, 90, bounds.size.w, 30));
  
  text_layer_set_background_color(week_day_layer, GColorBlack);
  text_layer_set_text_color(week_day_layer, GColorClear);
  text_layer_set_font(week_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(week_day_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(week_day_layer));
}

static void setup_week_number_layer(Layer *window_layer, GRect bounds) {
  week_numer_layer = text_layer_create(GRect(5, -5, 50, 30));
  
  text_layer_set_background_color(week_numer_layer, GColorBlack);
  text_layer_set_text_color(week_numer_layer, GColorClear);
  text_layer_set_font(week_numer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(week_numer_layer, GTextAlignmentLeft);
  
  layer_add_child(window_layer, text_layer_get_layer(week_numer_layer));
}

static void setup_main_window(Window *window) {
  window_set_background_color(window, GColorBlack);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  setup_week_number_layer(window_layer, bounds);
  setup_week_day_layer(window_layer, bounds);
  setup_date_layer(window_layer, bounds);
  setup_time_layer(window_layer, bounds);
  setup_battery_layer(window_layer, bounds);
  
  init_update_layers(time_layer, week_numer_layer, week_day_layer, date_layer, battery_layer);
}

static void tear_down_main_window(Window *window) {
  text_layer_destroy(week_numer_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(week_day_layer);
  layer_destroy(battery_layer);
}

static void on_message_received(DictionaryIterator *iter, void *context) {
  on_settings_changed(iter, context);
  update_all();
}

void setup_watchface() {
  main_window = window_create();
  
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = setup_main_window,
    .unload = tear_down_main_window
  });
  
  window_stack_push(main_window, true);
  
  update_all();
  
  tick_timer_service_subscribe(MINUTE_UNIT, on_time_tick);
  battery_state_service_subscribe(on_battery_state_changed);
  app_message_register_inbox_received(on_message_received);
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

void tear_down_watchface() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  app_message_register_inbox_received(NULL);
  window_destroy(main_window);
}