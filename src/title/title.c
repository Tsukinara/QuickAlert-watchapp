#include <pebble.h>
#include "title.h"

static GBitmap *s_button;
  
void title_draw(GContext* ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  if (!s_button) {
    s_button = gbitmap_create_with_resource(RESOURCE_ID_BUTTON_UP);
  }
  graphics_draw_bitmap_in_rect(ctx, s_button, (GRect) {.origin = {30, 58}, .size = {109, 90}});
  graphics_draw_text(ctx, "Hold down button\nuntil safe", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,0}, .size = {144,60}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void title_free() {
  if (s_button) { gbitmap_destroy(s_button); }
}