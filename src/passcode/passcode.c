#include <pebble.h>
#include "passcode.h"
  
static GBitmap *s_down;
static GBitmap *s_up;
static GBitmap *s_center;

void passcode_draw(GContext* ctx, char *passcode, int *timer, const int type) {
  graphics_context_set_text_color(ctx, GColorBlack);
  if (!s_down || !s_up || !s_center) {
    s_down = gbitmap_create_with_resource(RESOURCE_ID_DOWN);
    s_up = gbitmap_create_with_resource(RESOURCE_ID_UP);
    s_center = gbitmap_create_with_resource(RESOURCE_ID_CENTER);
  }
  
  int i, x;
  for (i = 0; i < (int)strlen(passcode); i++) {
    x = 26*i + 10;
    switch(passcode[i] - 48) {
      case 1:
        graphics_draw_bitmap_in_rect(ctx, s_up, (GRect) {.origin = {x, 65}, .size = {20, 20}});
        break;
      case 2:
        graphics_draw_bitmap_in_rect(ctx, s_center, (GRect) {.origin = {x, 65}, .size = {20, 20}});
        break;
      case 3:
        graphics_draw_bitmap_in_rect(ctx, s_down, (GRect) {.origin = {x, 65}, .size = {20, 20}});
        break;
    }
  }
  if (!type) {
    graphics_draw_text(ctx, "Enter passcode:", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,0}, .size = {144,25}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  } else {
    graphics_draw_text(ctx, "Invalid passcode.\nPlease try again:", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,0}, .size = {144,60}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  char *buffer = malloc(20);
  if (*timer < 10) {
    snprintf(buffer, 20, "00:0%d", *timer);
  } else {
    snprintf(buffer, 20, "00:%d", *timer);
  }
  free(buffer);
  graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS), (GRect) {.origin = {0,114}, .size = {144,36}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void passcode_free() {
  if (s_down) { gbitmap_destroy(s_down); }
  if (s_up) { gbitmap_destroy(s_up); }
  if (s_center) { gbitmap_destroy(s_center); }
}