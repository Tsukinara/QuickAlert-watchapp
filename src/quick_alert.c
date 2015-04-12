#include <pebble.h>
#include "title/title.h"
#include "button/button.h"
#include "passcode/passcode.h"
#include "error/error.h"
#include "call/call.h"
  
// universal constants
#define PAUSE_LEN 1000
#define BUFFER_LEN 30
#define PASSCODE_LEN 5
#define UPDATE_INTV 50
#define PASSCODE_PERSIST_KEY 17
#define TIMERLEN_PERSIST_KEY 18

// main display layer
static Layer *s_main_layer;

// app states:
// 0 - main screen
// 1 - button held
// 2 - passcode entry
// 3 - passcode success
// 4 - passcode failure
// 5 - call da police
// if passcode is not defined, only displays warning page
static int s_curr_state;
static bool s_passcode_defined;

// timer and counter tracking variables
static int s_timer_len;
static int passcode_counter;
static int s_timer_curr;

// various methods headers
static void s_display_unlock_msg(void *data);
static void s_display_error_msg(void *data);
static void s_call_police(void *data);
static void s_reset_app(void* data);
static bool s_check_code(void *data);

// passcode storage, 1 -> top, 2 -> mid, 3 -> bottom
static char s_passcode[PASSCODE_LEN + 1];
static char s_entry[PASSCODE_LEN + 1];

// app timer for countdown
static AppTimer *call_timer;

// inbox received callback - reads and interprets info from companion
//    key 0 -> new timer length
//    key 1 -> new passcode
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  while (t != NULL) {
    switch(t -> key) {
      case 0:
        s_timer_len = (t -> value -> uint8)*1000;
        persist_write_int(TIMERLEN_PERSIST_KEY, s_timer_len);
        break;
      case 1:
        snprintf(s_passcode, PASSCODE_LEN + 1, "%s", t -> value -> cstring);
        persist_write_string(PASSCODE_PERSIST_KEY, s_passcode);
        break;
    }
    t = dict_read_next(iterator);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
}

// inbox dropped callback - only logs the message drop
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

// outbox failed callback - only logs the message drop
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// outbox sent callback - only logs the message success
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// omain init - registers callback functions, reads persistent data, and requests most recent data from
// companion app. Sets default timer length to 10 seconds.
void main_init(void) {
  s_reset_app(NULL);
    
  // initialize app message
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
   
  // send request for most recent data
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, 0, 42);
  app_message_outbox_send();
  
  // gather persistent data for timer length and passcode
  if (persist_exists(TIMERLEN_PERSIST_KEY)) {
    s_timer_len = persist_read_int(TIMERLEN_PERSIST_KEY);
  } else {
    s_timer_len = 10*1000;
  }
  if (persist_exists(PASSCODE_PERSIST_KEY)) {
    s_passcode_defined = true;
    persist_read_string(PASSCODE_PERSIST_KEY, s_passcode, PASSCODE_LEN + 1);
  } else {
    s_passcode_defined = false;
  }
}

// mid button input handling
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_curr_state == 2 || s_curr_state == 4) {
    s_entry[passcode_counter] = '2';
    passcode_counter++;
  }
}

// up button input handling
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_curr_state == 2 || s_curr_state == 4) {
    s_entry[passcode_counter] = '1';
    passcode_counter++;
  }
}

// down button input handling
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_curr_state == 2 || s_curr_state == 4) {
    s_entry[passcode_counter] = '3';
    passcode_counter++;
  }
}

// disables back button on passcode screens
void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_curr_state != 2 && s_curr_state != 4) {
    window_stack_pop(true);
  }
}

// disables back button on passcode screens
void back_multi_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_curr_state != 2 && s_curr_state != 4) {
    window_stack_pop(true);
  }
}

// handles long mid hold
void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  // define short vibration pulse
  static const uint32_t segments[] = { 75 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
  
  if (s_curr_state == 0) {
    s_curr_state ++;
  }
}

// handles long mid hold release
void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  
  // define short vibration pulse
  static const uint32_t segments[] = { 75 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
  
  if (s_curr_state == 1) {
    // queue countdown for police calling
    call_timer = app_timer_register(s_timer_len, s_call_police, NULL);
    s_timer_curr = s_timer_len;
    s_curr_state ++;
  }
}

// check the accuracy of the passcode using strcmp
static bool s_check_code(void *data) {
  if(strcmp(s_passcode, s_entry) != 0) {
    return false;
  }
  return true;
}

// cancel the police call timer and returns the main screen
static void s_display_unlock_msg(void *data) {
  s_curr_state = 3;
  app_timer_register(PAUSE_LEN, s_reset_app, NULL);
  app_timer_cancel(call_timer);
}

// displays error message and re-prompts for password
static void s_display_error_msg(void *data) {  
  int i;
  vibes_double_pulse();
  s_curr_state = 4;
  passcode_counter = 0;
  for (i = 0; i < PASSCODE_LEN; i++) {
    s_entry[i] = ' ';
  }
}

// main renderer: called every cycle, and reads game state to draw different screens
// draws a 400 error if an unknown state is found, otherwise, just parses states
void main_render(Layer *layer, GContext *ctx) {
  int remaining;
  graphics_context_set_text_color(ctx, GColorBlack);
  if(!s_passcode_defined) {
    error_draw(ctx);
  } else switch (s_curr_state) {
      case 0:
        title_draw(ctx);
        break;
      case 1:
        button_draw(ctx);
        break;
      case 2:
        remaining = (int)(((double)s_timer_curr)/1000.0) + 1;
        passcode_draw(ctx, s_entry, &remaining, 0);
        break;
      case 3:
        graphics_draw_text(ctx, "Alert canceled", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,60}, .size = {144,25}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        break;
      case 4:
        remaining = (int)(((double)s_timer_curr)/1000.0) + 1;
        passcode_draw(ctx, s_entry, &remaining, 1);
        break;
      case 5:
        call_draw(ctx);
        break;
      default:
        graphics_draw_text(ctx, "400 Error:\n Bad Request", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,60}, .size = {144,60}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
}

// main updater: only useful in passcode states. Updates countdown timer, and constantly
// refreshes the screen otherwise
void main_update(void *data) {
  if (s_curr_state == 2 || s_curr_state == 4) {
    s_timer_curr -= UPDATE_INTV;
    if (passcode_counter == PASSCODE_LEN) {
      if (s_check_code(s_entry)) {
        s_display_unlock_msg(NULL);
      } else {
        s_display_error_msg(NULL);
      }
    }
  }
  layer_mark_dirty(s_main_layer);
  app_timer_register(UPDATE_INTV, main_update, data);
}


// main program start - changes the update proc of the layer to the renderer, which is called every run
// of game_update by layer_mark_dirty. Starts the first call of game_update
void main_start(Layer *layer) {
  s_main_layer = layer;
  layer_set_update_proc(s_main_layer, main_render);
  
  app_timer_register(UPDATE_INTV, main_update, NULL);
}

// calls da police. Sends a signal to the companion service, and pulses two vibrations.
static void s_call_police(void *data) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, 42, 42);
  app_message_outbox_send();
  app_timer_register(5*PAUSE_LEN, s_reset_app, NULL);
  
  // two long vibrations
  static const uint32_t segments[] = { 750, 250, 750 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
  
  s_curr_state = 5;
}


// resets the app back to the title screen
static void s_reset_app(void *data) {
  s_curr_state = 0;
  passcode_counter = 0;
  int i;
  for (i = 0; i < PASSCODE_LEN; i++) {
    s_entry[i] = ' ';
  }
}

// free all the images upon exit
void main_deinit() {
  title_free();
  error_free();
  button_free();
  call_free();
  passcode_free();
  s_reset_app(NULL);
}