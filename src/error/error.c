#include <pebble.h>
#include "error.h"

static GBitmap *s_warning;
  
void error_draw(GContext* ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  if (!s_warning) {
    s_warning = gbitmap_create_with_resource(RESOURCE_ID_WARNING);
  }
  
  graphics_draw_bitmap_in_rect(ctx, s_warning, (GRect) {.origin = {43, 8}, .size = {58, 56}});
  graphics_draw_text(ctx, "Please create a passcode prior\nto using app", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,64}, .size = {144,92}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}