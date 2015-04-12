#include <pebble.h>
#include "call.h"

static GBitmap *s_call;
  
void call_draw(GContext* ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  if (!s_call) {
    s_call = gbitmap_create_with_resource(RESOURCE_ID_CALL);
  }
  
  graphics_draw_bitmap_in_rect(ctx, s_call, (GRect) {.origin = {34, 9}, .size = {75, 75}});
  graphics_draw_text(ctx, "Location data sent\n Calling for help", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,84}, .size = {144,92}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void call_free() {
  if (s_call) { gbitmap_destroy(s_call); }
}