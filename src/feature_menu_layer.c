#include "pebble.h"

#define NUM_MENU_SECTIONS 2
#define MAX_SIZE 100
int size;
int ind;
TextLayer *text_layer;
bool inQ = true;
bool win = false;

enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};

typedef struct {
  char *name;
  char *room;
  //char *des;
}person;

person people[MAX_SIZE];

static Window *window1;
static Window *window2;

// This is a menu layer
// You have more control than with a simple menu layer
static MenuLayer *menu_layer;

// You can draw arbitrary things in a menu item such as a background
static GBitmap *menu_background;

// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_sections_callback2(MenuLayer *menu_layer, void *data) {
  return 1;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return size;
    default:
      return 0;
  }
}

static uint16_t menu_get_num_rows_callback2(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return 2;
    default:
      return 0;
  }
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_header_height_callback2(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT + 15;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Front of queue");
      break;

    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Back of queue");
      break;
  }
}

static void menu_draw_header_callback2(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    if(!inQ)
      menu_cell_basic_header_draw(ctx, cell_layer, "Add yourself to queue?");
    else
      menu_cell_basic_header_draw(ctx, cell_layer, "Remove yourself from queue?");
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  //people = 3;
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      if(cell_index->row < size){
          menu_cell_basic_draw(ctx, cell_layer, people[cell_index->row].name, people[cell_index->row].room, NULL);
      }
      break;
    }
}

static void menu_draw_row_callback2(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  //people = 3;
  switch (cell_index->row) {
    case 0:
      // Use the row to specify which item we'll draw
      menu_cell_basic_draw(ctx, cell_layer, "Yes", "", NULL);
      break;
    
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "No", "", NULL);
      break;
    
    }
}

void send_message(int n){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x1);
  if(n != -1)
    dict_write_int16(iter, MESSAGE_KEY, n);
  else if(inQ)
    dict_write_cstring(iter, MESSAGE_KEY, "Remove from queue");
  else
	  dict_write_cstring(iter, MESSAGE_KEY, "Add to queue");

	dict_write_end(iter);
  	app_message_outbox_send();
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

  window_stack_push(window2, true);

}

void menu_select_callback2(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  
  send_message(-1);
  if(cell_index->row == 0)
    inQ = !inQ;
  window_stack_pop(true);
}

// This initializes the menu upon window load
void window_load(Window *window) {
  
  // Here we load the bitmap assets
  // resource_init_current_app must be called before all asset loading

  // And also load the background
  menu_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_BRAINS);

  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  if(!win){
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
      .get_num_sections = menu_get_num_sections_callback,
      .get_num_rows = menu_get_num_rows_callback,
      .get_header_height = menu_get_header_height_callback,
      .draw_header = menu_draw_header_callback,
      .draw_row = menu_draw_row_callback,
      .select_click = menu_select_callback,
    });
    win = !win;
  }
  else
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
      .get_num_sections = menu_get_num_sections_callback2,
      .get_num_rows = menu_get_num_rows_callback2,
      .get_header_height = menu_get_header_height_callback2,
      .draw_header = menu_draw_header_callback2,
      .draw_row = menu_draw_row_callback2,
      .select_click = menu_select_callback2,
    });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

void window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(menu_layer);

  // And cleanup the background
  gbitmap_destroy(menu_background);
}

enum {
            AKEY_NUMBER,
            AKEY_TEXT,
        };

 void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %s", "Nope out");
 }

 void in_received_handler(DictionaryIterator *received, void *context) {
   // incoming message received
   // Check for fields you expect to receive

          // Act on the found fields received  
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %d", ind);
          if(ind == -1)
          {
            Tuple *num_tuple = dict_find(received, AKEY_NUMBER);
            size = num_tuple->value->int16;
            ind++;
            menu_layer_reload_data(menu_layer);
            return;
          }    
      
          Tuple *text_tuple = dict_find(received, AKEY_TEXT);
   
          if (text_tuple) {
              APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %s", text_tuple->value->cstring);
              people[ind].name = text_tuple->value->cstring;
              ind++;
              if(ind == size)
                ind = -1;
          }
          send_message(1); 
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %s", "Nope in");
 }

 static void init(void) {

   app_message_register_inbox_received(in_received_handler);
   app_message_register_inbox_dropped(in_dropped_handler);
   app_message_register_outbox_sent(out_sent_handler);
   app_message_register_outbox_failed(out_failed_handler);

   const uint32_t inbound_size = 64;
   const uint32_t outbound_size = 64;
   app_message_open(inbound_size, outbound_size);

 }

void deinit(void) {
	app_message_deregister_callbacks();
}

int main(void) {
  size = 1;
  people[0].name = "Nobody in queue";
  ind = -1;
  window1 = window_create();
  window2 = window_create();
  // Setup the window handlers

  init();
  window_set_window_handlers(window1, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_window_handlers(window2, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window1, true /* Animated */);

  app_event_loop();
  
  window_destroy(window2);
  window_destroy(window1);
  deinit();
}
