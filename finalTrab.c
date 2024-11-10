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
  int piece;
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
  t.piece = 0;
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
        al_draw_filled_rectangle(225 + x * TILE_SIZE, 125 + y * TILE_SIZE, 275 + x * TILE_SIZE, 175 + y * TILE_SIZE, al_map_rgb(150, 100, 50));
      }
      else if (x > 0 && y > 0 && x < 6 && y < 6)
      {
        drawTile(tile[x - 1][y - 1]);
        if (tile[x - 1][y - 1].piece == 1)
        {
          al_draw_filled_circle(250 + x * TILE_SIZE, 150 + y * TILE_SIZE, 20, al_map_rgb(200, 0, 0));
        }
        else if (tile[x - 1][y - 1].piece == 2)
        {
          al_draw_filled_circle(250 + x * TILE_SIZE, 150 + y * TILE_SIZE, 20, al_map_rgb(0, 0, 200));
        }
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
        tile[x][y] = createTile(id, 275 + x * TILE_SIZE, 175 + y * TILE_SIZE, 325 + x * TILE_SIZE, 225 + y * TILE_SIZE, TILE3_COLOR, HOVER);
      }
      else if ((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0))
      {
        tile[x][y] = createTile(id, 275 + x * TILE_SIZE, 175 + y * TILE_SIZE, 325 + x * TILE_SIZE, 225 + y * TILE_SIZE, TILE1_COLOR, HOVER);
      }
      else
      {
        tile[x][y] = createTile(id, 275 + x * TILE_SIZE, 175 + y * TILE_SIZE, 325 + x * TILE_SIZE, 225 + y * TILE_SIZE, TILE2_COLOR, HOVER);
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

void initializeMenuTexts(struct text texts[6], ALLEGRO_FONT *font)
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

int handleEvents(struct text texts[], bool *running, struct tile board[5][5], ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, int *round);
void startGame(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct tile board[5][5]);

void isHoveringTile(struct tile board[5][5], int mouse_x, int mouse_y, bool *hoveringTile)
{
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      board[i][j].hover = (mouse_x > board[i][j].x1 && mouse_x < board[i][j].x2 &&
                           mouse_y > board[i][j].y1 && mouse_y < board[i][j].y2);
      if (board[i][j].hover)
      {
        *hoveringTile = true;
      }
    }
  }
}

int mouseClickTile(struct tile board[5][5], int mouse_x, int mouse_y)
{
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      board[i][j].click = (mouse_x > board[i][j].x1 && mouse_x < board[i][j].x2 &&
                           mouse_y > board[i][j].y1 && mouse_y < board[i][j].y2);
      if (board[i][j].click)
      {
        return board[i][j].id;
      }
    }
  }
  return -1;
}

bool onMouseClickTile(ALLEGRO_DISPLAY **display, struct tile board[5][5], int mouse_x, int mouse_y, bool *running, int *round)
{
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      if (board[i][j].id == mouseClickTile(board, mouse_x, mouse_y) && board[i][j].id != 12 && board[i][j].piece == 0)
      {
        if (round != 0)
        {
          if (*round % 2 == 0)
          {
            board[i][j].piece = 1;
            return true;
          }
          else
          {
            board[i][j].piece = 2;
            return true;
          }
        }
      }
    }
  }
  return false;
}

void isHoveringText(struct text texts[], int mouse_x, int mouse_y, bool *hoveringText)
{
  for (size_t i = 0; i < 6; i++)
  {
    texts[i].hover = (mouse_x >= texts[i].x && mouse_x <= texts[i].x + texts[i].width &&
                      mouse_y >= texts[i].y && mouse_y <= texts[i].y + texts[i].height);
    if (texts[i].hover)
    {
      *hoveringText = true;
    }
  }
}

int mouseClickText(struct text texts[], int mouse_x, int mouse_y)
{
  for (size_t i = 0; i < 6; i++)
  {
    if (mouse_x >= texts[i].x && mouse_x <= texts[i].x + texts[i].width &&
        mouse_y >= texts[i].y && mouse_y <= texts[i].y + texts[i].height)
    {
      return texts[i].id;
    }
  }
  return -1;
}

void onMouseClickText(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct tile board[5][5], struct text texts[], int mouse_x, int mouse_y, bool *running)
{
  switch (mouseClickText(texts, mouse_x, mouse_y))
  {
  case 0:
    startGame(display, font, board);
    break;
  case 1:
    printf("Player V.S Computer\n");
    break;
  case 2:
    printf("Continue Last Game\n");
    break;
  case 3:
    printf("History\n");
    break;
  case 4:
    printf("Help\n");
    break;
  case 5:
    *running = false;
    break;
  default:
    break;
  }
}

void startGame(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct tile board[5][5])
{
  bool running = true;
  struct text texts[6] = {0};
  int round = 0;
  int pieces = 0;

  while (running)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));

    drawBoard(board);

    al_flip_display();

    if (handleEvents(texts, &running, board, display, font, &round) == 1)
    {
      pieces++;
      if (pieces % 2 == 0 && pieces != 0)
      {
        round++;
      }
    }
    printf("Round: %d\n", round);
  }
}

int handleEvents(struct text texts[], bool *running, struct tile board[5][5], ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, int *round)
{
  bool hoveringTile = false;
  bool hoveringText = false;
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(*display));
  al_register_event_source(event_queue, al_get_mouse_event_source());

  ALLEGRO_EVENT ev;
  al_wait_for_event(event_queue, &ev);

  if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
  {
    al_destroy_display(*display);
  }
  else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
  {
    int mouse_x = ev.mouse.x;
    int mouse_y = ev.mouse.y;
    isHoveringText(texts, mouse_x, mouse_y, &hoveringText);
    isHoveringTile(board, mouse_x, mouse_y, &hoveringTile);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      printf("Mouse click on text\n");
      onMouseClickText(display, font, board, texts, mouse_x, mouse_y, running);
      return 0;
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringTile)
    {
      if (onMouseClickTile(display, board, mouse_x, mouse_y, running, round))
      {
        return 1;
      }
    }
    al_destroy_event_queue(event_queue);
  }
  return -1;
}

void mouseEvents(struct text texts[], bool *running, struct tile board[5][5], ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font)
{
  handleEvents(texts, running, board, display, font, 0);
}

void menu(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font)
{
  bool running = true;
  struct text texts[6] = {0};

  struct tile board[5][5] = {0};
  initializeTiles(board);

  initializeMenuTexts(texts, *font);

  while (running)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));

    drawText(texts);

    al_flip_display();

    mouseEvents(texts, &running, board, display, font);
  }

  al_destroy_font(*font);
  al_destroy_display(*display);
}

int main()
{
  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_FONT *font = NULL;
  initializeAllegro(&display, &font);
  menu(&display, &font);
  return 0;
}
