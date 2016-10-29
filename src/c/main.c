#include <pebble.h>
#include <math.h>

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_week_numer_layer;
static TextLayer *s_week_day_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static int batteryIndicatorY = -1;
static int batteryIndicatorInitialX = -1;
static int batteryIndicatorSpace = 12;

static int lastWeekNumber = -1;
static int lastDayNumber = -1;
static int lastHourNumber = -1;

static int currentBatteryLevel = 5;

const char *days[] = {"Söndag", "Måndag", "Tisdag", "Onsdag", "Torsdag", "Fredag", "Lördag"}; 

static int get_current_week_number(struct tm *tick_time) {
  char s_week_number_buffer[3];  
  strftime(s_week_number_buffer, sizeof(s_week_number_buffer), "%W", tick_time);
  
  return atoi(s_week_number_buffer);
}

static void update_time(struct tm *tick_time) {
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);
}

static void update_date(struct tm *tick_time) {  
  static char s_date_buffer[11];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%Y-%m-%d", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void update_week_day(struct tm *tick_time) {
  text_layer_set_text(s_week_day_layer, days[tick_time->tm_wday]);
}

static void update_week(int currentWeekNumber) {
    static char s_week_text_buffer[5];
    snprintf(s_week_text_buffer, sizeof(s_week_text_buffer), "v.%d", currentWeekNumber);    
    text_layer_set_text(s_week_numer_layer, s_week_text_buffer);
}

static void handle_battery_state_changed(BatteryChargeState charge_state) {
  currentBatteryLevel = round(charge_state.charge_percent/20);
}

static void update_battery_level() {
  handle_battery_state_changed(battery_state_service_peek());
}

static void update_battery_layer(Layer *layer, GContext* ctx){
  graphics_context_set_fill_color(ctx, GColorClear);
  
  GPoint p = {.x = 0, .y = batteryIndicatorY};
  for(int i = 0; i < currentBatteryLevel; i++){
    p.x = batteryIndicatorInitialX - (i*batteryIndicatorSpace);
    graphics_fill_circle(ctx, p, 3);    
  }  
    
}

static void time_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  
  int currentHourNumber = tick_time->tm_hour;  
  if (lastHourNumber != currentHourNumber) {
    
    int currentDayNumber = tick_time->tm_yday;    
    if(lastDayNumber != currentDayNumber) {
      lastDayNumber = currentDayNumber;
      
      update_date(tick_time);
      update_week_day(tick_time);
      
      int currentWeekNumber = get_current_week_number(tick_time);
      if(lastWeekNumber != currentWeekNumber) {
        lastWeekNumber = currentWeekNumber;
      
        update_week(currentWeekNumber);
      }
      
    }
    
  }
  
}

static void time_layer_load(Layer *window_layer, GRect bounds) {
  s_time_layer = text_layer_create(GRect(0, 50, bounds.size.w, 50));
  
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void week_number_layer_load(Layer *window_layer, GRect bounds) {
  s_week_numer_layer = text_layer_create(GRect(5, -5, 50, 30));
  
  text_layer_set_background_color(s_week_numer_layer, GColorBlack);
  text_layer_set_text_color(s_week_numer_layer, GColorClear);
  text_layer_set_font(s_week_numer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_week_numer_layer, GTextAlignmentLeft);
  
  layer_add_child(window_layer, text_layer_get_layer(s_week_numer_layer));
}

static void date_layer_load(Layer *window_layer, GRect bounds) {
  s_date_layer = text_layer_create(GRect(0, 135, bounds.size.w, 30));
  
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void week_day_layer_load(Layer *window_layer, GRect bounds) {
  s_week_day_layer = text_layer_create(GRect(0, 90, bounds.size.w, 30));
  
  text_layer_set_background_color(s_week_day_layer, GColorBlack);
  text_layer_set_text_color(s_week_day_layer, GColorClear);
  text_layer_set_font(s_week_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_week_day_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_week_day_layer));
}

static void battery_layer_load(Layer *window_layer, GRect bounds) {
  s_battery_layer = layer_create(GRect(55, -5, bounds.size.w - 55, 30));
  GRect batteryLayerBounds = layer_get_bounds(s_battery_layer);
  
  batteryIndicatorY = batteryLayerBounds.origin.y + (batteryLayerBounds.size.h/2) + 2;
  batteryIndicatorInitialX = batteryLayerBounds.origin.x + batteryLayerBounds.size.w - 10;

  layer_set_update_proc(s_battery_layer, update_battery_layer);
  layer_add_child(window_layer, s_battery_layer);
}

static void main_window_load(Window *window) {
  window_set_background_color(window, GColorBlack);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  week_number_layer_load(window_layer, bounds);
  week_day_layer_load(window_layer, bounds);
  date_layer_load(window_layer, bounds);
  time_layer_load(window_layer, bounds);
  battery_layer_load(window_layer, bounds);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_week_numer_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_week_day_layer);
  layer_destroy(s_battery_layer);
}

static void init() {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  update_battery_level();
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  update_week(get_current_week_number(tick_time));
  update_time(tick_time);
  update_week_day(tick_time);
  update_date(tick_time);
  
  tick_timer_service_subscribe(MINUTE_UNIT, time_tick_handler);
  battery_state_service_subscribe(handle_battery_state_changed);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}