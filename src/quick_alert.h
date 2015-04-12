#include <pebble.h>

void select_click_handler(ClickRecognizerRef recognizer, void *context);
void up_click_handler(ClickRecognizerRef recognizer, void *context);
void down_click_handler(ClickRecognizerRef recognizer, void *context);
void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void back_click_handler(ClickRecognizerRef recognizer, void *context);
void back_multi_handler(ClickRecognizerRef recognizer, void *context);

void main_init(void);
void main_start(Layer *main_layer);