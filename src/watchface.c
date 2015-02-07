#include <pebble.h>

static Window *main_window;
static TextLayer *time_layer, *date_layer, *day_layer, *battery_layer;

int watch_still = 3;

static char time_buffer[6];

void update_display() {
	if (watch_still > 0) {
		text_layer_set_text(time_layer, time_buffer);
	} else {
		text_layer_set_text(time_layer, 0);
	}
}

void tick_handler(struct tm *time, TimeUnits units_changed) {
	static char date_buffer[11];
	static char day_buffer[10];
	if (watch_still > 0) watch_still -= 1;
	strftime(time_buffer, sizeof("00:00"), "%H:%M", time);
	strftime(date_buffer, sizeof(date_buffer), "%m.%d.%Y", time);
	strftime(day_buffer, sizeof(day_buffer), "%A", time);
	
	day_buffer[0] += 32;
	
	text_layer_set_text(date_layer, date_buffer);
	text_layer_set_text(day_layer, day_buffer);
	update_display();
}

static void data_handler(AccelData *data, uint32_t num_samples) {
	int accel_level = 200;
	int diff_X = 0;
	int diff_Y = 0;
	int diff_Z = 0;
	if (!data[2].did_vibrate && !data[1].did_vibrate && !data[0].did_vibrate) {
		diff_X = abs(data[2].x) - abs(data[0].x);
		diff_Y = abs(data[2].y) - abs(data[0].y);
		diff_Z = abs(data[2].z) - abs(data[0].z);
	}
	if (diff_X > accel_level || diff_Y > accel_level || diff_Z > accel_level) watch_still = 2;
	update_display();
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char battery_buffer[32];
  snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", new_state.charge_percent);
  text_layer_set_text(battery_layer, battery_buffer);
}

static void init(void) {
	main_window = window_create();
	Layer *window_layer = window_get_root_layer(main_window);
	window_set_background_color(main_window, GColorBlack);
	
	time_layer = text_layer_create(GRect(0, 71, 144, 43));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	text_layer_set_text_color(time_layer, GColorWhite);
	
	date_layer = text_layer_create(GRect(0, 123, 144, 26));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_text_color(date_layer, GColorWhite);
		
	day_layer = text_layer_create(GRect(0, 0, 144, 32));
	text_layer_set_background_color(day_layer, GColorClear);
	text_layer_set_font(day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_LIGHT_20)));
	text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
	text_layer_set_text_color(day_layer, GColorWhite);
	
	battery_layer = text_layer_create(GRect(0, 40, 144, 30));
	text_layer_set_background_color(battery_layer, GColorClear);
	text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
	text_layer_set_text_color(battery_layer, GColorWhite);
	
	battery_handler(battery_state_service_peek());
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	uint32_t num_samples = 3;
	accel_data_service_subscribe(num_samples, data_handler);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	
	battery_state_service_subscribe(battery_handler);
	
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));	
	layer_add_child(window_layer, text_layer_get_layer(day_layer));
	layer_add_child(window_layer, text_layer_get_layer(battery_layer));

	window_stack_push(main_window, true);
 }

static void deinit(void) {
	window_destroy(main_window);
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(day_layer);
	text_layer_destroy(battery_layer);
	tick_timer_service_unsubscribe();
	accel_data_service_unsubscribe();
	battery_state_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
