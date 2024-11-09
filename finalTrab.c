#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 50
#define TILE1_COLOR al_map_rgb(0, 0, 0)
#define TILE2_COLOR al_map_rgb(100, 100, 100)
#define TILE3_COLOR al_map_rgb(255, 122, 255)
#define HOVER al_map_rgb(255, 0, 0)
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
        al_draw_filled_rectangle(50 + x * TILE_SIZE, 50 + y * TILE_SIZE, 100 + x * TILE_SIZE, 100 + y * TILE_SIZE, al_map_rgb(150, 100, 50));
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
        tile[x][y] = createTile(id, 100 + x * TILE_SIZE, 100 + y * TILE_SIZE, 150 + x * TILE_SIZE, 150 + y * TILE_SIZE, TILE3_COLOR, HOVER);
      }
      else if ((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0))
      {
        tile[x][y] = createTile(id, 100 + x * TILE_SIZE, 100 + y * TILE_SIZE, 150 + x * TILE_SIZE, 150 + y * TILE_SIZE, TILE1_COLOR, HOVER);
      }
      else
      {
        tile[x][y] = createTile(id, 100 + x * TILE_SIZE, 100 + y * TILE_SIZE, 150 + x * TILE_SIZE, 150 + y * TILE_SIZE, TILE2_COLOR, HOVER);
      }
      id++;
    }
  }
}

struct text
{
  int id;
  int x;
  int y;
  int width;
  int height;
  char text[100];
  ALLEGRO_COLOR color;
  ALLEGRO_COLOR colorHover;
  ALLEGRO_FONT *font;
  bool hover;
  bool click;
};

int getCenter(ALLEGRO_FONT *font, const char *text)
{
  int text_x = SCREEN_WIDTH / 2 - al_get_text_width(font, text) / 2;
  return text_x;
}

struct text createText(int id, int y, const char *text, ALLEGRO_COLOR color, ALLEGRO_COLOR colorHover, ALLEGRO_FONT *font)
{
  struct text t;
  t.id = id;
  t.x = getCenter(font, text);
  t.y = y;
  t.width = al_get_text_width(font, text);
  t.height = al_get_font_line_height(font);
  strncpy(t.text, text, sizeof(t.text) - 1);
  t.text[sizeof(t.text) - 1] = '\0';
  t.color = color;
  t.colorHover = colorHover;
  t.font = font;
  t.hover = false;
  t.click = false;
  return t;
}

void initializeTexts(struct text texts[6], ALLEGRO_FONT *font)
{
  texts[0] = createText(0, 100, "Player V.S Player", al_map_rgb(0, 0, 0), HOVER, font);
  texts[1] = createText(1, 150, "Player V.S Computer", al_map_rgb(0, 0, 0), HOVER, font);
  texts[2] = createText(2, 200, "Continue Last Game", al_map_rgb(0, 0, 0), HOVER, font);
  texts[3] = createText(3, 250, "History", al_map_rgb(0, 0, 0), HOVER, font);
  texts[4] = createText(4, 300, "Help", al_map_rgb(0, 0, 0), HOVER, font);
  texts[5] = createText(5, 350, "Quit", al_map_rgb(0, 0, 0), HOVER, font);
}

void drawText(struct text texts[])
{
  for (size_t i = 0; i < 6; i++)
  {
    if (texts[i].hover)
    {
      al_draw_text(texts[i].font, texts[i].colorHover, texts[i].x, texts[i].y, 0, texts[i].text);
    }
    else
    {
      al_draw_text(texts[i].font, texts[i].color, texts[i].x, texts[i].y, 0, texts[i].text);
    }
  }
}

bool initializeAllegro(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font)
{
  if (!al_init())
  {
    fprintf(stderr, "Failed to initialize Allegro.\n");
    return false;
  }

  if (!al_install_mouse())
  {
    fprintf(stderr, "Failed to initialize mouse.\n");
    return false;
  }

  if (!al_init_font_addon())
  {
    fprintf(stderr, "Failed to initialize font addon.\n");
    return false;
  }

  if (!al_init_ttf_addon())
  {
    fprintf(stderr, "Failed to initialize TTF addon.\n");
    return false;
  }

  if (!al_init_primitives_addon())
  {
    fprintf(stderr, "Failed to initialize primitives addon.\n");
    return false;
  }

  *display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
  if (!*display)
  {
    fprintf(stderr, "Failed to create display.\n");
    return false;
  }

  *font = al_load_ttf_font("MinecraftRegular-Bmg3.otf", 32, 0);
  if (!*font)
  {
    fprintf(stderr, "Failed to load font. Make sure 'MinecraftRegular-Bmg3.otf' is in the correct directory.\n");
    al_destroy_display(*display);
    return false;
  }

  return true;
}

void menu(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font)
{

  // Text position and content
  const char *text = "Click Me!";

  bool running = true;
  bool hovering = false;
  struct tile board[5][5] = {0};
  struct text texts[6] = {0};
  initializeTiles(board);

  initializeTexts(texts, *font);

  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(*display));
  al_register_event_source(event_queue, al_get_mouse_event_source());

  while (running)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));

    drawText(texts);

    al_flip_display();

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
      int text_width = al_get_text_width(*font, text);
      int text_height = al_get_font_line_height(*font);

      // hovering = (mouse_x >= text_x && mouse_x <= text_x + text_width &&
      //             mouse_y >= text_y && mouse_y <= text_y + text_height);

      if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hovering)
      {
        running = false;
      }
    }
  }

  al_destroy_font(*font);
  al_destroy_display(*display);
  al_destroy_event_queue(event_queue);
}

int main()
{
  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_FONT *font = NULL;
  initializeAllegro(&display, &font);

  menu(&display, &font);
  return 0;
}
