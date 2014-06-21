#include <pebble.h>
  Window *window;
  MenuLayer *menu_layer;

void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
   // incoming message received
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }

void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
  switch(cell_index->row) {
    case 0:
    menu_cell_basic_draw(ctx, cell_layer, "Red", "Make the table red!", NULL);
    break;
    case 1:
    menu_cell_basic_draw(ctx, cell_layer, "Orange", "Make the table orange!", NULL);
    break;
    case 2:
    menu_cell_basic_draw(ctx, cell_layer, "Yellow", "Make the table yellow!", NULL);
    break;
    case 3:
    menu_cell_basic_draw(ctx, cell_layer, "Green", "Make the table green!", NULL);
    break;
    case 4:
    menu_cell_basic_draw(ctx, cell_layer, "Blue", "Make the table blue!", NULL);
    break;
    case 5:
    menu_cell_basic_draw(ctx, cell_layer, "Purple", "Make the table purple!", NULL);
    break;
    case 6:
    menu_cell_basic_draw(ctx, cell_layer, "Rainbow Pattern", "Rainbows!", NULL);
    break;
    case 7:
    menu_cell_basic_draw(ctx, cell_layer, "Random Pattern", "Flash random colors!", NULL);
    break;
    case 8:
    menu_cell_basic_draw(ctx, cell_layer, "Cylon Pattern", "Cylons!", NULL);
    break;
    case 9:
    menu_cell_basic_draw(ctx, cell_layer, "Off", "Turn the table off", NULL);
    break;
  }
}

static void send_cmd(uint8_t cmd) {
  Tuplet value = TupletInteger(0, cmd);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_outbox_send();
}

uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
  return 9;
}
 
void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
  int which = cell_index->row;
  send_cmd(which);
}
  void window_load(Window *window) {
    menu_layer = menu_layer_create(GRect(0,0,144,168-16));
    
    menu_layer_set_click_config_onto_window(menu_layer, window);
    
    MenuLayerCallbacks callbacks = {
      .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
      .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
      .select_click = (MenuLayerSelectCallback) select_click_callback
    };
    menu_layer_set_callbacks(menu_layer, NULL, callbacks);
    
    layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));
  }

  void window_unload(Window *window) {
    menu_layer_destroy(menu_layer);
  }
  void init() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_stack_push(window, true);
    const uint32_t inbound_size = 64;
   const uint32_t outbound_size = 64;
    app_message_register_inbox_received(in_received_handler);
   app_message_register_inbox_dropped(in_dropped_handler);
   app_message_register_outbox_sent(out_sent_handler);
   app_message_register_outbox_failed(out_failed_handler);
   app_message_open(inbound_size, outbound_size);
  }

  void deinit() {
    window_destroy(window);
  }

  int main(void) {
    init();
    app_event_loop();
    deinit();
  }