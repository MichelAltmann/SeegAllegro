#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

struct tile
{
  int id;
  int x1;
  int y1;
  int x2;
  int y2;
  ALLEGRO_COLOR color;
  ALLEGRO_COLOR colorHover;
  bool hover;
  bool click;
};

struct tile createTile(int id, int x1, int y1, int x2, int y2, ALLEGRO_COLOR color, ALLEGRO_COLOR colorHover)
{
  struct tile t;
  t.id = id;
  t.x1 = x1;
  t.y1 = y1;
  t.x2 = x2;
  t.y2 = y2;
  t.color = color;
  t.colorHover = colorHover;
  t.hover = false;
  t.click = false;
  return t;
}

void drawTile(struct tile tile)
{
  if (tile.hover)
  {
    al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, tile.colorHover);
  }
  else
  {
    al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, tile.color);
  }
}

void drawBoard(struct tile tile[5][5])
{
  for (size_t x = 0; x <= 6; x++)
  {
    for (size_t y = 0; y <= 6; y++)
    {
      if (x == 0 || y == 0 || x == 6 || y == 6)
      {
        al_draw_filled_rectangle(50 + x * 50, 50 + y * 50, 100 + x * 50, 100 + y * 50, al_map_rgb(150, 100, 50));
      }
      else if (x > 0 && y > 0 && x < 6 && y < 6)
      {
        drawTile(tile[x - 1][y - 1]);
      }
    }
  }
}

void initializeTiles(struct tile tile[5][5])
{
  int id = 0;
  for (size_t x = 0; x < 5; x++)
  {
    for (size_t y = 0; y < 5; y++)
    {
      if (x == 2 && y == 2)
      {
        tile[x][y] = createTile(id, 100 + x * 50, 100 + y * 50, 150 + x * 50, 150 + y * 50, al_map_rgb(255, 122, 255), al_map_rgb(255, 122, 255));
      }
      else if ((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0))
      {
        tile[x][y] = createTile(id, 100 + x * 50, 100 + y * 50, 150 + x * 50, 150 + y * 50, al_map_rgb(0, 0, 0), al_map_rgb(0, 0, 0));
      }
      else
      {
        tile[x][y] = createTile(id, 100 + x * 50, 100 + y * 50, 150 + x * 50, 150 + y * 50, al_map_rgb(100, 100, 100), al_map_rgb(100, 100, 100));
      }
      id++;
    }
  }
}

int main()
{
  if (!al_init())
  {
    fprintf(stderr, "Failed to initialize Allegro.\n");
    return -1;
  }

  if (!al_install_mouse())
  {
    fprintf(stderr, "Failed to initialize mouse.\n");
    return -1;
  }

  if (!al_init_font_addon())
  {
    fprintf(stderr, "Failed to initialize font addon.\n");
    return -1;
  }

  if (!al_init_ttf_addon())
  {
    fprintf(stderr, "Failed to initialize TTF addon.\n");
    return -1;
  }

  if (!al_init_primitives_addon())
  {
    fprintf(stderr, "Failed to initialize primitives addon.\n");
    return -1;
  }

  // Create display
  ALLEGRO_DISPLAY *display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
  if (!display)
  {
    fprintf(stderr, "Failed to create display.\n");
    return -1;
  }

  // Load font
  ALLEGRO_FONT *font = al_load_ttf_font("MinecraftRegular-Bmg3.otf", 32, 0);
  if (!font)
  {
    fprintf(stderr, "Failed to load font. Make sure 'MinecraftRegular-Bmg3.otf' is in the correct directory.\n");
    al_destroy_display(display);
    return -1;
  }

  // Set up event queue
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_mouse_event_source());

  // Text position and content
  const char *text = "Click Me!";
  int text_x = SCREEN_WIDTH / 2 - al_get_text_width(font, text) / 2;
  int text_y = SCREEN_HEIGHT / 2;

  bool running = true;
  bool hovering = false; // Track if mouse is hovering over the text
  struct tile board[5][5] = {0};
  initializeTiles(board);

  while (running)
  {
    // Draw the text with different colors based on hover state
    al_clear_to_color(al_map_rgb(255, 255, 255));
    drawBoard(board);
    if (hovering)
    {
      al_draw_text(font, al_map_rgb(255, 0, 0), text_x, text_y, 0, text); // Red when hovered
    }
    else
    {
      al_draw_text(font, al_map_rgb(255, 255, 255), text_x, text_y, 0, text); // White otherwise
    }

    al_flip_display();

    // Wait for events
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);

    if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
    {
      running = false;
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
    {
      int mouse_x = ev.mouse.x;
      int mouse_y = ev.mouse.y;
      int text_width = al_get_text_width(font, text);
      int text_height = al_get_font_line_height(font);

      // Check if the mouse is within text bounds for hover
      hovering = (mouse_x >= text_x && mouse_x <= text_x + text_width &&
                  mouse_y >= text_y && mouse_y <= text_y + text_height);

      if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hovering)
      {
        // Text was clicked
        printf("Text clicked!\n");
      }
    }
  }

  // Clean up
  al_destroy_font(font);
  al_destroy_display(display);
  al_destroy_event_queue(event_queue);
  return 0;
}
