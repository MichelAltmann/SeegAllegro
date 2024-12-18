#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200
#define TILE_SIZE 75
#define TEXT 36
#define SMALL_TEXT 30
#define TITLE_TEXT 60
#define CENTER_BOARD_X (SCREEN_WIDTH / 2) - (TILE_SIZE * 5 / 2)
#define CENTER_BOARD_Y (SCREEN_HEIGHT / 2) - (TILE_SIZE * 5 / 2)
#define TILE_END_X CENTER_BOARD_X + TILE_SIZE
#define TILE_END_Y CENTER_BOARD_Y + TILE_SIZE
#define TILE1_COLOR al_map_rgb(0, 0, 0)
#define TILE2_COLOR al_map_rgb(100, 100, 100)
#define TILE3_COLOR al_map_rgb(255, 122, 255)
#define HOVER al_map_rgb(255, 0, 50)
#define HOVER2 al_map_rgb(50, 0, 255)

typedef enum
{
  MAIN_MENU,
  HELP,
  GAME,
  GAME_COMPUTER,
  LOAD_GAME,
  LOAD_GAME_COMPUTER,
  HISTORY
} GameState;

struct tile
{
  int id;
  int x;
  int y;
  int x1;
  int y1;
  int x2;
  int y2;
  ALLEGRO_COLOR color;
  bool hover;
  bool click;
  bool positionable;
  bool canEat;
  int piece;
};

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

typedef struct
{
  int turns;
  double time;
  int winner;
  int result;
  bool isComputer;
} Game;

typedef struct
{
  int x;
  int y;
} Coordinates;

Game *readGameHistory(const char *filename, int *gameCount)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error opening file!\n");
    return NULL;
  }

  Game *games = NULL;
  *gameCount = 0;
  char line[100];

  while (fgets(line, sizeof(line), file))
  {
    if (strncmp(line, "Turns:", 6) == 0)
    {
      games = realloc(games, (*gameCount + 1) * sizeof(Game));
      sscanf(line, "Turns: %d", &games[*gameCount].turns);
    }
    else if (strncmp(line, "Time:", 5) == 0)
    {
      sscanf(line, "Time: %lf", &games[*gameCount].time);
    }
    else if (strncmp(line, "Winner:", 7) == 0)
    {
      sscanf(line, "Winner: %d", &games[*gameCount].winner);
      (*gameCount)++;
    }
    else if (strncmp(line, "Result:", 7) == 0)
    {
      sscanf(line, "Result: %d", &games[*gameCount].result);
    }
    else if (strncmp(line, "Computer:", 9) == 0)
    {
      int isComputer;
      sscanf(line, "Computer: %d", &isComputer);
      games[*gameCount - 1].isComputer = isComputer;
    }
  }

  fclose(file);
  return games;
}

struct tile createTile(int id, int x, int y, int x1, int y1, int x2, int y2, ALLEGRO_COLOR color)
{
  struct tile t;
  t.id = id;
  t.x = x;
  t.y = y;
  t.x1 = x1;
  t.y1 = y1;
  t.x2 = x2;
  t.y2 = y2;
  t.color = color;
  t.hover = false;
  t.click = false;
  t.positionable = false;
  t.canEat = false;
  t.piece = 0;
  return t;
}

bool initializeAllegro(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
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

  *font = al_load_ttf_font("MinecraftRegular-Bmg3.otf", TEXT, 0);
  if (!*font)
  {
    fprintf(stderr, "Failed to load font. Make sure 'MinecraftRegular-Bmg3.otf' is in the correct directory.\n");
    al_destroy_display(*display);
    return false;
  }

  *smallFont = al_load_ttf_font("MinecraftRegular-Bmg3.otf", SMALL_TEXT, 0);
  if (!*smallFont)
  {
    fprintf(stderr, "Failed to load font. Make sure 'MinecraftRegular-Bmg3.otf' is in the correct directory.\n");
    al_destroy_display(*display);
    return false;
  }

  return true;
}

ALLEGRO_EVENT_QUEUE *setupEventQueue(ALLEGRO_DISPLAY **display)
{
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(*display));
  al_register_event_source(event_queue, al_get_mouse_event_source());
  return event_queue;
}

int getCenter(ALLEGRO_FONT *font, const char *text)
{
  int text_x = SCREEN_WIDTH / 2 - al_get_text_width(font, text) / 2;
  return text_x;
}

struct text createText(int id, int x, int y, const char *text, ALLEGRO_COLOR color, ALLEGRO_COLOR colorHover, ALLEGRO_FONT *font)
{
  struct text t;
  t.id = id;
  t.x = x;
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

int getCenterY(ALLEGRO_FONT *font, const char *text, int y, int height)
{
  int text_y = SCREEN_HEIGHT / 2 - height / 2;
  return text_y;
}

void initializeMenuTexts(struct text texts[7], ALLEGRO_FONT *font)
{
  int height = 19 * al_get_font_line_height(font);
  int y = getCenterY(font, "Player V.S Player", 100, height);
  texts[0] = createText(0, getCenter(font, "Player V.S Player"), y, "Player V.S Player", al_map_rgb(0, 0, 0), HOVER, font);
  y += 2 * al_get_font_line_height(font);
  texts[1] = createText(1, getCenter(font, "Player V.S Computer"), y, "Player V.S Computer", al_map_rgb(0, 0, 0), HOVER, font);
  y += 2 * al_get_font_line_height(font);
  texts[2] = createText(2, getCenter(font, "Continue vs Player"), y, "Continue vs Player", al_map_rgb(0, 0, 0), HOVER, font);
  y += 2 * al_get_font_line_height(font);
  texts[3] = createText(3, getCenter(font, "Continue vs Computer"), y, "Continue vs Computer", al_map_rgb(0, 0, 0), HOVER, font);
  y += 2 * al_get_font_line_height(font);
  texts[4] = createText(4, getCenter(font, "History"), y, "History", al_map_rgb(0, 0, 0), HOVER, font);
  y += 2 * al_get_font_line_height(font);
  texts[5] = createText(5, getCenter(font, "Help"), y, "Help", al_map_rgb(0, 0, 0), HOVER, font);
  y += 2 * al_get_font_line_height(font);
  texts[6] = createText(6, getCenter(font, "Quit"), y, "Quit", al_map_rgb(0, 0, 0), HOVER, font);
}

void drawText(struct text texts[], int length)
{
  for (size_t i = 0; i < length; i++)
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

bool isHoveringText(struct text texts[], int mouse_x, int mouse_y, int size)
{
  for (size_t i = 0; i < size; i++)
  {
    texts[i].hover = (mouse_x >= texts[i].x && mouse_x <= texts[i].x + texts[i].width &&
                      mouse_y >= texts[i].y && mouse_y <= texts[i].y + texts[i].height);
    if (texts[i].hover)
    {
      return true;
    }
  }
  return false;
}

int mouseClickText(struct text texts[], int mouse_x, int mouse_y, int size)
{
  for (size_t i = 0; i < size; i++)
  {
    if (mouse_x >= texts[i].x && mouse_x <= texts[i].x + texts[i].width &&
        mouse_y >= texts[i].y && mouse_y <= texts[i].y + texts[i].height)
    {
      return texts[i].id;
    }
  }
  return -1;
}

void onMouseClickMenuText(struct text texts[], int mouse_x, int mouse_y, bool *running, bool *menuRunning, GameState *currentState)
{
  switch (mouseClickText(texts, mouse_x, mouse_y, 7))
  {
  case 0:
    *menuRunning = false;
    *currentState = GAME;
    break;
  case 1:
    *menuRunning = false;
    *currentState = GAME_COMPUTER;
    break;
  case 2:
    *menuRunning = false;
    *currentState = LOAD_GAME;
    break;
  case 3:
    *menuRunning = false;
    *currentState = LOAD_GAME_COMPUTER;
    break;
  case 4:
    *menuRunning = 0;
    *currentState = HISTORY;
    break;
  case 5:
    *menuRunning = false;
    *currentState = HELP;
    break;
  case 6:
    *menuRunning = false;
    *running = false;
    break;
  default:
    break;
  }
}

int handleMenuEvents(struct text texts[], bool *running, bool *menuRunning, ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont, GameState *currentState)
{
  bool hoveringText = false;
  ALLEGRO_EVENT_QUEUE *event_queue = setupEventQueue(display);
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
    hoveringText = isHoveringText(texts, mouse_x, mouse_y, 7);

    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      onMouseClickMenuText(texts, mouse_x, mouse_y, running, menuRunning, currentState);
      return 0;
    }
    al_destroy_event_queue(event_queue);
  }
  return -1;
}

void menu(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont, GameState *currentState, bool *running)
{
  struct text texts[7] = {0};

  bool menuRunning = true;
  initializeMenuTexts(texts, *font);

  while (menuRunning)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));

    ALLEGRO_FONT *titleFont = al_load_ttf_font("MinecraftRegular-Bmg3.otf", TITLE_TEXT, 0);

    al_draw_text(titleFont, al_map_rgb(200, 0, 0), getCenter(titleFont, "SeegAllegro"), 50, 0, "Seeg");
    al_draw_text(titleFont, al_map_rgb(150, 0, 150), getCenter(titleFont, "SeegAllegro") + al_get_text_width(titleFont, "Seeg"), 50, 0, "A");
    al_draw_text(titleFont, al_map_rgb(0, 0, 200), getCenter(titleFont, "SeegAllegro") + al_get_text_width(titleFont, "SeegA"), 50, 0, "llegro");

    drawText(texts, sizeof(texts) / sizeof(texts[0]));

    al_flip_display();

    handleMenuEvents(texts, running, &menuRunning, display, font, smallFont, currentState);
  }
}

void saveGameState(struct tile board[5][5], int turn, char *filename, int pieces, double currentTime)
{
  FILE *file = fopen(filename, "w");
  if (file == NULL)
  {
    printf("Error opening file!\n");
    return;
  }
  fprintf(file, "%d\n", turn);
  fprintf(file, "%d\n", pieces);
  fprintf(file, "%f\n", currentTime);
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      fprintf(file, "%d ", board[i][j].piece);
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

void saveGameOnHistory(int turn, double currentTime, int winner, bool isComputer, int winType)
{
  FILE *file = fopen("history.txt", "a");
  if (file == NULL)
  {
    printf("Error opening file!\n");
    return;
  }
  fprintf(file, "Turns: %d\n", turn);
  fprintf(file, "Time: %.2f\n", currentTime);
  fprintf(file, "Result: %d\n", winType);
  fprintf(file, "Winner: %d\n", winner);
  fprintf(file, "Computer: %d\n", isComputer);

  fclose(file);
}

void loadGameState(struct tile board[5][5], int *turn, char *filename, int *pieces, double *currentTime)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error opening file!\n");
    return;
  }
  fscanf(file, "%d", turn);
  fscanf(file, "%d", pieces);
  fscanf(file, "%lf", currentTime);
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      fscanf(file, "%d", &board[i][j].piece);
    }
  }
  fclose(file);
}

void onMouseClickPauseMenuText(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct text texts[], int mouse_x, int mouse_y, GameState *currentState, bool *isPaused, bool *gameRunning, struct tile board[5][5], int turn, int pieces, double currentTime, bool *saved, bool isComputer)
{
  switch (mouseClickText(texts, mouse_x, mouse_y, 3))
  {
  case 0:
    *isPaused = false;
    break;
  case 1:
    if (isComputer)
    {
      saveGameState(board, turn, "game_save_computer.txt", pieces, currentTime);
    }
    else
    {
      saveGameState(board, turn, "game_save.txt", pieces, currentTime);
    }
    *saved = true;
    break;
  case 2:
    *isPaused = false;
    *gameRunning = false;
    *currentState = MAIN_MENU;
    break;
  default:
    break;
  }
}

void handlePauseMenuEvents(struct text texts[], ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, GameState *currentState, bool *isPaused, bool *gameRunning, struct tile board[5][5], int turn, int pieces, double currentTime, bool *saved, bool isComputer)
{
  bool hoveringText = false;
  ALLEGRO_EVENT_QUEUE *event_queue = setupEventQueue(display);
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
    hoveringText = isHoveringText(texts, mouse_x, mouse_y, 3);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      onMouseClickPauseMenuText(display, font, texts, mouse_x, mouse_y, currentState, isPaused, gameRunning, board, turn, pieces, currentTime, saved, isComputer);
    }
    al_destroy_event_queue(event_queue);
  }
}

void initializePauseMenuTexts(struct text texts[], ALLEGRO_FONT *font)
{
  char resume[10] = "Resume";
  char saveGame[10] = "Save Game";
  char exit[10] = "Exit";
  float y = 60;

  texts[0] = createText(0, getCenter(font, resume), y, resume, al_map_rgb(0, 0, 0), HOVER, font);
  y += 1.2 * al_get_font_line_height(font);
  texts[1] = createText(1, getCenter(font, saveGame), y, saveGame, al_map_rgb(0, 0, 0), HOVER, font);
  y += 1.2 * al_get_font_line_height(font);
  texts[2] = createText(2, getCenter(font, exit), y, exit, al_map_rgb(0, 0, 0), HOVER, font);
}

void pauseMenu(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct text texts[], GameState *currentState, bool *isPaused, bool *gameRunning, size_t textLength, struct tile board[5][5], int turn, int pieces, double currentTime, bool *saved, bool isComputer)
{
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, "GAME IS PAUSED"), 10, 0, "GAME IS PAUSED");

  drawText(texts, textLength);

  al_flip_display();

  if (*saved)
  {
    al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, "Game saved"), SCREEN_HEIGHT - 40, 0, "Game saved");
  }

  handlePauseMenuEvents(texts, display, font, currentState, isPaused, gameRunning, board, turn, pieces, currentTime, saved, isComputer);
}

void onMouseClickGameOverText(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct text texts[], int mouse_x, int mouse_y, bool *running, GameState *currentState, bool isComputer)
{
  switch (mouseClickText(texts, mouse_x, mouse_y, 2))
  {
  case 0:
    *running = false;
    *currentState = isComputer ? GAME_COMPUTER : GAME;
    break;
  case 1:
    *running = false;
    *currentState = MAIN_MENU;
    break;
  default:
    break;
  }
}

int handleGameOverEvents(struct text texts[], ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, bool *running, GameState *currentState, bool isComputer)
{
  ALLEGRO_EVENT_QUEUE *event_queue = setupEventQueue(display);
  ALLEGRO_EVENT ev;
  bool hoveringText = false;
  al_wait_for_event(event_queue, &ev);
  if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
  {
    al_destroy_display(*display);
  }
  else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
  {
    int mouse_x = ev.mouse.x;
    int mouse_y = ev.mouse.y;

    hoveringText = isHoveringText(texts, mouse_x, mouse_y, 2);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      onMouseClickGameOverText(display, font, texts, mouse_x, mouse_y, running, currentState, isComputer);
    }
  }
  return -1;
}

void drawGameOver(int *winner, ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, GameState *currentState, bool isComputer)
{
  bool gameOverRunning = true;
  char text[20];
  char text1[20] = "Play Again";
  char text2[20] = "Back to Menu";
  struct text texts[2] = {0};
  int y = 200 + al_get_font_line_height(*font);
  texts[0] = createText(0, getCenter(*font, text1), y, text1, al_map_rgb(0, 0, 0), HOVER, *font);
  y += 4 * al_get_font_line_height(*font);
  texts[1] = createText(1, getCenter(*font, text2), y, text2, al_map_rgb(0, 0, 0), HOVER, *font);
  y = 200 + al_get_font_line_height(*font);

  sprintf(text, "Player %d Wins!", *winner);
  y += 2 * al_get_font_line_height(*font);
  while (gameOverRunning)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, "GAME OVER"), 120, 0, "GAME OVER");
    if (*winner != 3)
    {
      al_draw_textf(*font, *winner == 2 ? al_map_rgb(0, 0, 200) : al_map_rgb(200, 0, 0), getCenter(*font, text), y, 0, "%s", text);
    }
    else
    {
      al_draw_text(*font, al_map_rgb(150, 0, 150), getCenter(*font, "Draw!"), y, 0, "Draw!");
    }

    drawText(texts, sizeof(texts) / sizeof(texts[0]));

    handleGameOverEvents(texts, display, font, &gameOverRunning, currentState, isComputer);

    al_flip_display();
  }
  *winner = 0;
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
        tile[x][y] = createTile(id, x, y, CENTER_BOARD_X + x * TILE_SIZE, CENTER_BOARD_Y + y * TILE_SIZE, TILE_END_X + x * TILE_SIZE, TILE_END_Y + y * TILE_SIZE, TILE3_COLOR);
      }
      else if ((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0))
      {
        tile[x][y] = createTile(id, x, y, CENTER_BOARD_X + x * TILE_SIZE, CENTER_BOARD_Y + y * TILE_SIZE, TILE_END_X + x * TILE_SIZE, TILE_END_Y + y * TILE_SIZE, TILE1_COLOR);
      }
      else
      {
        tile[x][y] = createTile(id, x, y, CENTER_BOARD_X + x * TILE_SIZE, CENTER_BOARD_Y + y * TILE_SIZE, TILE_END_X + x * TILE_SIZE, TILE_END_Y + y * TILE_SIZE, TILE2_COLOR);
      }
      id++;
    }
  }
}

void drawTile(struct tile tile, int turn)
{
  if (tile.hover)
  {
    al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, turn % 2 == 0 ? HOVER : HOVER2);
  }
  else if (tile.positionable)
  {
    if (tile.canEat)
    {
      al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, al_map_rgb(200, 200, 0));
    }
    else
    {
      al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, al_map_rgba(0, 255, 0, 100));
    }
  }
  else
  {
    al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, tile.color);
  }
}

void drawBoard(struct tile tile[5][5], int turn)
{
  for (size_t x = 0; x <= 6; x++)
  {
    for (size_t y = 0; y <= 6; y++)
    {
      if (x == 0 || y == 0 || x == 6 || y == 6)
      {
        al_draw_filled_rectangle((CENTER_BOARD_X - TILE_SIZE) + x * TILE_SIZE, (CENTER_BOARD_Y - TILE_SIZE) + y * TILE_SIZE, (TILE_END_X - TILE_SIZE) + x * TILE_SIZE, (TILE_END_Y - TILE_SIZE) + y * TILE_SIZE, al_map_rgb(150, 100, 50));
      }
      else if (x > 0 && y > 0 && x < 6 && y < 6)
      {
        drawTile(tile[x - 1][y - 1], turn);
        if (tile[x - 1][y - 1].piece == 1)
        {
          al_draw_filled_circle((CENTER_BOARD_X - (TILE_SIZE / 2)) + x * TILE_SIZE, (CENTER_BOARD_Y - (TILE_SIZE / 2)) + y * TILE_SIZE, TILE_SIZE / 3, al_map_rgb(200, 0, 0));
        }
        else if (tile[x - 1][y - 1].piece == 2)
        {
          al_draw_filled_circle((CENTER_BOARD_X - (TILE_SIZE / 2)) + x * TILE_SIZE, (CENTER_BOARD_Y - (TILE_SIZE / 2)) + y * TILE_SIZE, TILE_SIZE / 3, al_map_rgb(0, 0, 200));
        }
      }
    }
  }
}

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

void clearPositionable(struct tile board[5][5])
{
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      board[i][j].positionable = false;
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
        board[i][j].click = false;
        return board[i][j].id;
      }
    }
  }
  return -1;
}

void putPiece(struct tile board[5][5], int i, int j, int *turn)
{
  if (turn != 0)
  {
    if (*turn % 2 == 0)
    {
      board[i][j].piece = 1;
    }
    else
    {
      board[i][j].piece = 2;
    }
  }
}

bool checkTilePossibilities(struct tile board[5][5], int i, int j, bool *tileUp, bool *tileDown, bool *tileLeft, bool *tileRight)
{
  if (j - 1 >= 0)
  {
    *tileUp = (board[i][j - 1].piece == 0);
  }
  else
  {
    *tileUp = false;
  }
  if (j + 1 < 5)
  {
    *tileDown = (board[i][j + 1].piece == 0);
  }
  else
  {
    *tileDown = false;
  }
  if (i + 1 < 5)
  {
    *tileRight = (board[i + 1][j].piece == 0);
  }
  else
  {
    *tileRight = false;
  }
  if (i - 1 >= 0)
  {
    *tileLeft = (board[i - 1][j].piece == 0);
  }
  else
  {
    *tileLeft = false;
  }

  return (*tileUp || *tileDown || *tileLeft || *tileRight);
}

bool isValidPosition(int x, int y, struct tile board[5][5], int piece)
{
  return (x >= 0 && x < 5 && y >= 0 && y < 5) && (board[x][y].piece != piece);
}

int checkEatCoordinates(struct tile board[5][5], int x, int y, int playerPiece, int opponentPiece, Coordinates eatenCoords[4])
{
  int count = 0;

  if (x + 2 < 5)
  {
    if (board[x + 1][y].piece == opponentPiece && board[x + 1][y].id != 12 && board[x + 2][y].piece == playerPiece)
    {
      eatenCoords[count].x = x + 1;
      eatenCoords[count].y = y;
      count++;
    }
  }

  if (x - 2 >= 0)
  {
    if (board[x - 1][y].piece == opponentPiece && board[x - 1][y].id != 12 && board[x - 2][y].piece == playerPiece)
    {
      eatenCoords[count].x = x - 1;
      eatenCoords[count].y = y;
      count++;
    }
  }

  if (y + 2 < 5)
  {
    if (board[x][y + 1].piece == opponentPiece && board[x][y + 1].id != 12 && board[x][y + 2].piece == playerPiece)
    {
      eatenCoords[count].x = x;
      eatenCoords[count].y = y + 1;
      count++;
    }
  }

  if (y - 2 >= 0)
  {
    if (board[x][y - 1].piece == opponentPiece && board[x][y - 1].id != 12 && board[x][y - 2].piece == playerPiece)
    {
      eatenCoords[count].x = x;
      eatenCoords[count].y = y - 1;
      count++;
    }
  }

  return count;
}

void setTilePossibility(struct tile board[5][5], int i, int j, bool hasPossibilities)
{
  bool tileUp, tileDown, tileLeft, tileRight;
  clearPositionable(board);
  int opponentPiece = board[i][j].piece == 1 ? 2 : 1;
  if (checkTilePossibilities(board, i, j, &tileUp, &tileDown, &tileLeft, &tileRight) && hasPossibilities)
  {
    if (j - 1 >= 0)
    {
      Coordinates eatenCoords[4];
      int count = checkEatCoordinates(board, i, j - 1, board[i][j].piece, opponentPiece, eatenCoords);
      board[i][j - 1].canEat = count > 0;
      board[i][j - 1].positionable = tileUp;
    }
    if (j + 1 < 5)
    {
      Coordinates eatenCoords[4];
      int count = checkEatCoordinates(board, i, j + 1, board[i][j].piece, opponentPiece, eatenCoords);
      board[i][j + 1].canEat = count > 0;
      board[i][j + 1].positionable = tileDown;
    }
    if (i - 1 >= 0)
    {
      Coordinates eatenCoords[4];
      int count = checkEatCoordinates(board, i - 1, j, board[i][j].piece, opponentPiece, eatenCoords);
      board[i - 1][j].canEat = count > 0;
      board[i - 1][j].positionable = tileLeft;
    }
    if (i + 1 < 5)
    {
      Coordinates eatenCoords[4];
      int count = checkEatCoordinates(board, i + 1, j, board[i][j].piece, opponentPiece, eatenCoords);
      board[i + 1][j].canEat = count > 0;
      board[i + 1][j].positionable = tileRight;
    }
  }
  else if (!hasPossibilities)
  {
    int piece = board[i][j].piece;
    if (j - 1 >= 0)
    {
      board[i][j - 1].positionable = isValidPosition(i, j - 1, board, piece);
    }
    if (j + 1 < 5)
    {
      board[i][j + 1].positionable = isValidPosition(i, j + 1, board, piece);
    }
    if (i - 1 >= 0)
    {
      board[i - 1][j].positionable = isValidPosition(i - 1, j, board, piece);
    }
    if (i + 1 < 5)
    {
      board[i + 1][j].positionable = isValidPosition(i + 1, j, board, piece);
    }
  }
  else
  {
    clearPositionable(board);
  }
}

bool checkPossibilities(struct tile board[5][5], int turn)
{
  bool tileUp, tileDown, tileLeft, tileRight;
  int possibilities = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if (board[i][j].piece == 1 && turn % 2 == 0 && turn >= 12)
      {
        if (checkTilePossibilities(board, i, j, &tileUp, &tileDown, &tileLeft, &tileRight))
        {
          possibilities++;
        }
      }
      else if (board[i][j].piece == 2 && turn % 2 != 0 && turn >= 12)
      {
        if (checkTilePossibilities(board, i, j, &tileUp, &tileDown, &tileLeft, &tileRight))
        {
          possibilities++;
        }
      }
    }
  }
  return possibilities > 0;
}

bool eatPieces(struct tile board[5][5], Coordinates eatenCoords[4], int count)
{
  for (int i = 0; i < count; i++)
  {
    board[eatenCoords[i].x][eatenCoords[i].y].piece = 0;
  }
  if (count > 0)
  {
    return true;
  }
  return false;
}

void checkSmallWinPieces(int upPiece1, int upPiece2, int downPiece1, int downPiece2, bool *running, int turn, bool isXwin, int *winner, int *winType)
{
  if (upPiece1 >= 0 && upPiece2 == 0 && downPiece1 == 0 && downPiece2 >= 0)
  {
    if (turn % 2 == 0)
    {
      *winner = 1;
      *winType = 2;
    }
    else
    {
      *winner = 2;
      *winType = 2;
    }
  }
  else if (upPiece1 == 0 && upPiece2 >= 0 && downPiece1 >= 0 && downPiece2 == 0)
  {
    if (turn % 2 == 0)
    {
      *winner = 1;
      *winType = 2;
    }
    else
    {
      *winner = 2;
      *winType = 2;
    }
  }
}

void checkSmallWinSides(struct tile board[5][5], int x, int y, bool *running, int turn, bool isXwin, int *winner, int *winType)
{
  int upPiece1 = 0;
  int upPiece2 = 0;
  int downPiece1 = 0;
  int downPiece2 = 0;
  if (isXwin)
  {
    for (int i = 0; i < 5; i++)
    {
      for (int j = 0; j < y; j++)
      {
        if (board[i][j].piece == 1)
        {
          upPiece1++;
        }
        else if (board[i][j].piece == 2)
        {
          upPiece2++;
        }
      }
    }
    for (int i = 0; i < 5; i++)
    {
      for (int j = y + 1; j < 5; j++)
      {
        if (board[i][j].piece == 1)
        {
          downPiece1++;
        }
        else if (board[i][j].piece == 2)
        {
          downPiece2++;
        }
      }
    }
    checkSmallWinPieces(upPiece1, upPiece2, downPiece1, downPiece2, running, turn, isXwin, winner, winType);
  }
  else
  {
    for (int i = 0; i < x; i++)
    {
      for (int j = 0; j < 5; j++)
      {
        if (board[i][j].piece == 1)
        {
          upPiece1++;
        }
        else if (board[i][j].piece == 2)
        {
          upPiece2++;
        }
      }
    }
    for (int i = x + 1; i < 5; i++)
    {
      for (int j = 0; j < 5; j++)
      {
        if (board[i][j].piece == 1)
        {
          downPiece1++;
        }
        else if (board[i][j].piece == 2)
        {
          downPiece2++;
        }
      }
    }
    checkSmallWinPieces(upPiece1, upPiece2, downPiece1, downPiece2, running, turn, isXwin, winner, winType);
  }
}

void checkSmallWin(struct tile board[5][5], int x, int y, bool *running, int turn, int *winner, int *winType)
{
  int xwin = 0;
  int ywin = 0;
  for (int i = 0; i < 5; i++)
  {
    if (turn % 2 == 0)
    {
      if (board[i][y].piece == 1)
      {
        xwin++;
      }
    }
    else
    {
      if (board[i][y].piece == 2)
      {
        xwin++;
      }
    }
  }

  if (xwin == 5 && y != 0 && y != 4)
  {
    checkSmallWinSides(board, x, y, running, turn, true, winner, winType);
  }

  for (int i = 0; i < 5; i++)
  {
    if (turn % 2 == 0)
    {
      if (board[x][i].piece == 1)
      {
        ywin++;
      }
    }
    else
    {
      if (board[x][i].piece == 2)
      {
        ywin++;
      }
    }
  }

  if (ywin == 5 && x != 0 && x != 4)
  {
    checkSmallWinSides(board, x, y, running, turn, false, winner, winType);
  }
}

void checkWin(struct tile board[5][5], int turn, bool *running, int *winner, int *winType)
{
  int player1 = 0;
  int player2 = 0;
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      if (board[i][j].piece == 1)
      {
        player1++;
      }
      else if (board[i][j].piece == 2)
      {
        player2++;
      }
    }
  }
  if (player1 == 0)
  {
    *winner = 2;
    *winType = 1;
  }
  else if (player2 == 0)
  {
    *winner = 1;
    *winType = 1;
  }
}

void checkDraw(struct tile board[5][5], int turn, bool *running, int *winner, int *winType)
{
  int player1 = 0;
  int player2 = 0;
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      if (board[i][j].piece == 1)
      {
        player1++;
      }
      else if (board[i][j].piece == 2)
      {
        player2++;
      }
    }
  }
  if (player1 <= 3 && player2 <= 3)
  {
    *winner = 3;
    *winType = 3;
  }
}

void movePiece(struct tile board[5][5], int x, int y, struct tile *selectedTile, bool *isConsecutiveTurn)
{
  board[x][y].piece = selectedTile->piece;

  if (!*isConsecutiveTurn)
  {
    selectedTile->id = -1;
  }
  board[selectedTile->x][selectedTile->y].piece = 0;
  clearPositionable(board);
}

void canEatCoordinates(struct tile board[5][5], int x, int y, int *turn, bool *left, bool *right, bool *up, bool *down)
{
  if (x + 1 < 5)
  {
    Coordinates eatenCoords[4];
    if (board[x + 1][y].piece == 0)
    {
      int count = checkEatCoordinates(board, x + 1, y, *turn % 2 == 0 ? 1 : 2, *turn % 2 == 0 ? 2 : 1, eatenCoords);
      if (count > 0)
      {
        *right = true;
      }
    }
  }
  if (x - 1 >= 0)
  {
    Coordinates eatenCoords[4];
    if (board[x - 1][y].piece == 0)
    {
      int count = checkEatCoordinates(board, x - 1, y, *turn % 2 == 0 ? 1 : 2, *turn % 2 == 0 ? 2 : 1, eatenCoords);
      if (count > 0)
      {
        *left = true;
      }
    }
  }
  if (y + 1 < 5)
  {
    Coordinates eatenCoords[4];
    if (board[x][y + 1].piece == 0)
    {
      int count = checkEatCoordinates(board, x, y + 1, *turn % 2 == 0 ? 1 : 2, *turn % 2 == 0 ? 2 : 1, eatenCoords);
      if (count > 0)
      {
        *down = true;
      }
    }
  }
  if (y - 1 >= 0)
  {
    Coordinates eatenCoords[4];
    if (board[x][y - 1].piece == 0)
    {
      int count = checkEatCoordinates(board, x, y - 1, *turn % 2 == 0 ? 1 : 2, *turn % 2 == 0 ? 2 : 1, eatenCoords);
      if (count > 0)
      {
        *up = true;
      }
    }
  }
}

bool onMouseClickTile(struct tile board[5][5], int mouse_x, int mouse_y, bool *running, int *turn, struct tile *selectedTile, bool hasPossibilities, int *winner, int *winType, bool *isConsecutiveTurn)
{
  int clickedTileId = mouseClickTile(board, mouse_x, mouse_y);
  for (size_t i = 0; i < 5; i++)
  {
    for (size_t j = 0; j < 5; j++)
    {
      if (board[i][j].id == clickedTileId)
      {
        if (board[i][j].id != 12 && board[i][j].piece == 0 && *turn < 12)
        {
          putPiece(board, i, j, turn);
          return true;
        }
        else if (board[i][j].piece == 1 && *turn >= 12 && *turn % 2 == 0 && !*isConsecutiveTurn)
        {
          setTilePossibility(board, i, j, hasPossibilities);
          *selectedTile = board[i][j];
        }
        else if (board[i][j].piece == 2 && *turn >= 12 && *turn % 2 != 0 && !*isConsecutiveTurn)
        {
          setTilePossibility(board, i, j, hasPossibilities);
          *selectedTile = board[i][j];
        }
        else if (board[i][j].positionable && selectedTile->id != -1)
        {
          movePiece(board, i, j, selectedTile, isConsecutiveTurn);
          Coordinates eatenCoords[4];
          int count = checkEatCoordinates(board, i, j, *turn % 2 == 0 ? 1 : 2, *turn % 2 == 0 ? 2 : 1, eatenCoords);
          if (eatPieces(board, eatenCoords, count))
          {
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
            canEatCoordinates(board, i, j, turn, &left, &right, &up, &down);
            if (left || right || up || down)
            {
              board[i + 1][j].positionable = right;
              board[i + 1][j].canEat = right;
              board[i - 1][j].positionable = left;
              board[i - 1][j].canEat = left;
              board[i][j + 1].positionable = down;
              board[i][j + 1].canEat = down;
              board[i][j - 1].positionable = up;
              board[i][j - 1].canEat = up;
              *selectedTile = board[i][j];
              *isConsecutiveTurn = true;
              return false;
            }
            else
            {
              *isConsecutiveTurn = false;
            }
          }
          checkDraw(board, *turn, running, winner, winType);
          checkWin(board, *turn, running, winner, winType);
          checkSmallWin(board, i, j, running, *turn, winner, winType);
          return true;
        }
      }
    }
  }
  return false;
}

int handleGameEvents(struct text texts[], bool *running, ALLEGRO_DISPLAY **display, struct tile board[5][5], ALLEGRO_FONT **font, int *round, struct tile *selectedTile, bool hasPossibilities, GameState *currentState, bool *isPaused, double *currentTime, time_t startTime, int *winner, int *winType, bool *isConsecutiveTurn)
{
  bool hoveringTile = false;
  bool hoveringText = false;
  ALLEGRO_TIMER *timer = al_create_timer(1.0 / 20.0);
  ALLEGRO_EVENT_QUEUE *event_queue = setupEventQueue(display);
  al_register_event_source(event_queue, al_get_timer_event_source(timer));
  ALLEGRO_EVENT ev;
  al_start_timer(timer);
  al_wait_for_event(event_queue, &ev);
  if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
  {
    al_destroy_display(*display);
  }
  else if (ev.type == ALLEGRO_EVENT_TIMER)
  {
    *currentTime = difftime(time(NULL), startTime);
  }
  else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
  {
    int mouse_x = ev.mouse.x;
    int mouse_y = ev.mouse.y;

    isHoveringTile(board, mouse_x, mouse_y, &hoveringTile);
    hoveringText = isHoveringText(texts, mouse_x, mouse_y, 1);
    ;
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringTile)
    {
      if (onMouseClickTile(board, mouse_x, mouse_y, running, round, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn))
      {
        return 1;
      }
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      *isPaused = true;
    }
  }
  al_destroy_timer(timer);
  return -1;
}

void checkTilePositionables(struct tile board[5][5], int x, int y, bool *up, bool *down, bool *left, bool *right, bool hasPossibilities)
{
  if (x - 1 >= 0)
  {
    if (hasPossibilities)
    {
      *left = board[x - 1][y].positionable;
    }
    else if (board[x - 1][y].piece == 1)
    {
      *left = true;
    }
  }
  if (x + 1 < 5)
  {
    if (hasPossibilities)
    {
      *right = board[x + 1][y].positionable;
    }
    else if (board[x + 1][y].piece == 1)
    {
      *right = true;
    }
  }
  if (y - 1 >= 0)
  {
    if (hasPossibilities)
    {
      *up = board[x][y - 1].positionable;
    }
    else if (board[x][y - 1].piece == 1)
    {
      *up = true;
    }
  }
  if (y + 1 < 5)
  {
    if (hasPossibilities)
    {
      *down = board[x][y + 1].positionable;
    }
    else if (board[x][y + 1].piece == 1)
    {
      *down = true;
    }
  }
}

void computerMove(struct tile board[5][5], bool *running, int *turn, struct tile *selectedTile, bool hasPossibilities, int *winner, int *winType, bool *isConsecutiveTurn, bool right, bool left, bool up, bool down)
{
  if (right)
  {
    onMouseClickTile(board, selectedTile->x2 + 5, selectedTile->y1 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
  }
  if (left)
  {
    onMouseClickTile(board, selectedTile->x1 - 5, selectedTile->y1 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
  }
  if (down)
  {
    onMouseClickTile(board, selectedTile->x1 + 5, selectedTile->y2 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
  }
  if (up)
  {
    onMouseClickTile(board, selectedTile->x1 + 5, selectedTile->y1 - 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
  }
}

void computerTurn(struct tile board[5][5], bool *running, int *turn, struct tile *selectedTile, bool hasPossibilities, int *winner, int *winType, bool *isConsecutiveTurn)
{
  int x, y;
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  if (*turn < 12)
  {
    do
    {
      x = rand() % 5;
      y = rand() % 5;
    } while (board[x][y].piece != 0 || board[x][y].id == 12);
    onMouseClickTile(board, board[x][y].x1 + 5, board[x][y].y1 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
  }
  else
  {
    usleep(300000);
    bool pieceFound = false;
    if (selectedTile->id == -1 && hasPossibilities)
    {
      for (x = 0; x < 5 && !pieceFound; x++)
      {
        for (y = 0; y < 5 && !pieceFound; y++)
        {
          if (board[x][y].piece == 2)
          {
            canEatCoordinates(board, x, y, turn, &left, &right, &up, &down);
            if (left || right || up || down)
            {
              pieceFound = true;
              onMouseClickTile(board, board[x][y].x1 + 5, board[x][y].y1 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
            }
          }
        }
      }
      while (!pieceFound)
      {
        do
        {
          x = rand() % 5;
          y = rand() % 5;
        } while (board[x][y].piece != 2);
        checkTilePossibilities(board, x, y, &up, &down, &left, &right);
        if (left || right || up || down)
        {
          pieceFound = true;
          onMouseClickTile(board, board[x][y].x1 + 5, board[x][y].y1 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
        }
      }
    }
    else
    {
      while (!pieceFound)
      {
        do
        {
          x = rand() % 5;
          y = rand() % 5;
        } while (board[x][y].piece != 2);

        onMouseClickTile(board, board[x][y].x1 + 5, board[x][y].y1 + 5, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn);
        checkTilePositionables(board, selectedTile->x, selectedTile->y, &up, &down, &left, &right, hasPossibilities);
        if (left || right || up || down)
        {
          pieceFound = true;
        }
      }
    }
    if (*isConsecutiveTurn)
    {
      checkTilePositionables(board, selectedTile->x, selectedTile->y, &up, &down, &left, &right, hasPossibilities);
      computerMove(board, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn, right, left, up, down);
      *isConsecutiveTurn = false;
    }
    computerMove(board, running, turn, selectedTile, hasPossibilities, winner, winType, isConsecutiveTurn, right, left, up, down);
  }
}

void game(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, bool *running, GameState *currentState, int turn, int pieces, struct tile board[5][5], double savedTime, bool isComputer, int *winner)
{
  bool gameRunning = true;
  struct text texts[1] = {0};
  struct text textsMenu[3] = {0};
  texts[0] = createText(0, SCREEN_WIDTH - al_get_text_width(*font, "Pause"), 20, "Pause", al_map_rgb(0, 0, 0), HOVER, *font);
  struct tile selectedTile = {0};
  bool isConsecutiveTurn = false;
  bool hasPossibilities = false;
  bool isPaused = false;
  bool saved = false;
  initializePauseMenuTexts(textsMenu, *font);
  time_t startTime = time(NULL);
  double elapsedTime = 0;
  double currentTime = savedTime;
  int winType = 0;

  if (savedTime != 0)
  {
    startTime = time(NULL) - currentTime;
    elapsedTime = currentTime;
  }
  while (gameRunning)
  {
    if (isPaused)
    {
      pauseMenu(display, font, textsMenu, currentState, &isPaused, &gameRunning, sizeof(textsMenu) / sizeof(textsMenu[0]), board, turn, pieces, currentTime, &saved, isComputer);
      if (!isPaused)
      {
        startTime = time(NULL) - currentTime;
        saved = false;
      }
    }
    else
    {
      currentTime = difftime(time(NULL), startTime);
      elapsedTime = currentTime;
      al_clear_to_color(al_map_rgb(255, 255, 255));
      drawText(texts, sizeof(texts) / sizeof(texts[0]));
      drawBoard(board, turn);
      int y = 10;
      al_draw_text(*font, al_map_rgb(0, 0, 0), 10, y, 0, "Turn: ");
      al_draw_textf(*font, turn % 2 == 0 ? al_map_rgb(200, 0, 0) : al_map_rgb(0, 0, 200), al_get_text_width(*font, "Turn: "), y, 0, "%s", turn % 2 == 0 ? "Player 1" : "Player 2");
      y += al_get_font_line_height(*font);
      al_draw_textf(*font, al_map_rgb(0, 0, 0), 10, y, 0, "Time: %.0f", currentTime);
      y += al_get_font_line_height(*font);
      al_draw_textf(*font, al_map_rgb(0, 0, 0), 10, y, 0, turn < 12 ? "Placement Phase" : "Movement Phase");

      al_flip_display();

      hasPossibilities = checkPossibilities(board, turn);

      if (isComputer && turn % 2 != 0)
      {
        computerTurn(board, &gameRunning, &turn, &selectedTile, hasPossibilities, winner, &winType, &isConsecutiveTurn);
        pieces++;
        if (pieces % 2 == 0 && pieces != 0 && pieces < 24)
        {
          turn++;
        }
        else if (pieces >= 24 && !isConsecutiveTurn)
        {
          turn++;
          clearPositionable(board);
        }
      }
      else if (handleGameEvents(texts, running, display, board, font, &turn, &selectedTile, hasPossibilities, currentState, &isPaused, &currentTime, startTime, winner, &winType, &isConsecutiveTurn) == 1)
      {
        pieces++;
        if (pieces % 2 == 0 && pieces != 0 && pieces < 24)
        {
          turn++;
        }
        else if (pieces >= 24)
        {
          turn++;
        }
      }
      if (*winner > 0)
      {
        saveGameOnHistory(turn, elapsedTime, *winner, isComputer, winType);
        gameRunning = false;
        drawGameOver(winner, display, font, currentState, isComputer);
      }
    }
  }
}

void onMouseClickHelpText(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct text texts[], int mouse_x, int mouse_y, bool *running, bool *helpRunning, int *page, GameState *currentState)
{
  switch (mouseClickText(texts, mouse_x, mouse_y, 3))
  {
  case 0:
    *helpRunning = false;
    *currentState = MAIN_MENU;
    break;
  case 1:
    if (*page > 0)
    {
      *page = *page - 1;
    }
    break;
  case 2:
    if (*page < 11)
    {
      *page = *page + 1;
    }
    break;
  default:
    break;
  }
}

int handleHelpEvents(struct text texts[], bool *running, bool *helpRunning, ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, int *page, GameState *currentState)
{
  bool hoveringText = false;
  ALLEGRO_EVENT_QUEUE *event_queue = setupEventQueue(display);
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
    hoveringText = isHoveringText(texts, mouse_x, mouse_y, 3);

    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      onMouseClickHelpText(display, font, texts, mouse_x, mouse_y, running, helpRunning, page, currentState);
      return 0;
    }
    al_destroy_event_queue(event_queue);
  }
  return -1;
}

void page0(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "The game is played on a 5x5 board.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
}

void page1(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "Each player has 12 pieces,";
  char text1[100] = "player 1 is red, and player 2 is blue.";
  board[1][2].piece = 1;
  board[3][2].piece = 2;
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
}

void page2(struct tile board[5][5], ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  initializeTiles(board);
  char text[100] = "Placement phase:";
  char text1[100] = "Players take turns placing 2 of their pieces on the board each turn.";
  char text2[100] = "players CANNOT place their pieces in the middle during this phase.";
  float y = 30;
  board[2][2].color = al_map_rgb(255, 0, 0);
  board[4][2].piece = 1;
  board[3][1].piece = 2;
  board[1][2].piece = 1;
  board[3][2].piece = 2;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text1), y, 0, text1);
  y += 1.2 * al_get_font_line_height(*smallFont);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text2), y, 0, text2);
}

void adjacentSpaces(struct tile board[5][5])
{
  board[4][2].piece = 1;
  board[3][1].piece = 2;
  board[1][2].piece = 1;
  board[3][2].piece = 2;
  board[0][2].positionable = true;
  board[1][1].positionable = true;
  board[1][3].positionable = true;
  board[2][2].positionable = true;
}

void page3(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "Movement phase:";
  char text1[100] = "Players take turns moving their pieces to adjacent empty spaces.";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  adjacentSpaces(board);
}

void page4(struct tile board[5][5], ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  initializeTiles(board);
  char text[100] = "Objective:";
  char text1[100] = "The objective is to eat all the opponent's pieces.";
  char text2[100] = "You can eat an opponent's piece by surrounding it with your pieces in your turn.";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text2), y, 0, text2);
  adjacentSpaces(board);
}

void page5(struct tile board[5][5], ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  initializeTiles(board);
  char text[100] = "Objective:";
  char text1[100] = "The objective is to eat all the opponent's pieces.";
  char text2[100] = "You can eat an opponent's piece by surrounding it with your pieces in your turn.";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text2), y, 0, text2);
  board[4][2].piece = 1;
  board[3][1].piece = 2;
  board[2][2].piece = 1;
}

void page6(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "You CANNOT eat an opponents piece";
  char text1[100] = "if it's in the center of the board.";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  board[2][2].piece = 2;
  board[2][2].color = al_map_rgb(255, 0, 0);
  board[3][2].piece = 1;
  board[1][2].piece = 1;
}

void stuckPieces(struct tile board[5][5])
{
  board[0][0].piece = 1;
  board[0][1].piece = 2;
  board[1][0].piece = 2;
  board[4][4].piece = 1;
  board[4][3].piece = 2;
  board[3][4].piece = 2;
  board[4][0].piece = 1;
  board[3][0].piece = 2;
  board[4][1].piece = 2;
  board[0][4].piece = 1;
  board[0][3].piece = 2;
  board[1][4].piece = 2;

  board[0][3].positionable = true;
  board[1][4].positionable = true;
}

void page7(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "If stuck with no empty spaces to move.";
  char text1[100] = "You can eat an opponent's piece.";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  stuckPieces(board);
}

void page8(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "If stuck with no empty spaces to move.";
  char text1[100] = "You can eat an opponent's piece.";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  stuckPieces(board);
  board[0][4].piece = 0;
  board[1][4].piece = 1;
  board[0][3].positionable = false;
  board[1][4].positionable = false;
}

void page9(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "You win by eating all the opponent's pieces.";
  char text1[100] = "Player 2 Wins!";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 200), getCenter(*font, text1), SCREEN_HEIGHT - al_get_font_line_height(*font), 0, text1);
  board[3][2].piece = 2;
  board[1][4].piece = 2;
  board[1][3].piece = 2;
  board[4][3].piece = 2;
}

void page10(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "If each player has 3 or less pieces.";
  char text1[100] = "The game draws!";
  char text2[100] = "Draw!";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  al_draw_text(*font, al_map_rgb(200, 0, 200), getCenter(*font, text2), SCREEN_HEIGHT - al_get_font_line_height(*font), 0, text2);
  board[3][2].piece = 2;
  board[1][4].piece = 2;
  board[1][3].piece = 2;
  board[2][0].piece = 1;
  board[0][4].piece = 1;
  board[4][3].piece = 1;
}

void page11(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "You can have a small win by aligning all your pieces in a row or column";
  char text1[100] = "separating the board in two, with all the opponents pieces on one side.";
  char text2[100] = "Player 1 Small win!";
  float y = 30;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), y, 0, text);
  y += 1.2 * al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), y, 0, text1);
  al_draw_text(*font, al_map_rgb(200, 0, 0), getCenter(*font, text2), SCREEN_HEIGHT - al_get_font_line_height(*font), 0, text2);
  board[0][0].piece = 1;
  board[0][1].piece = 1;
  board[1][2].piece = 1;
  board[1][3].piece = 1;
  board[0][4].piece = 1;
  board[2][0].piece = 1;
  board[2][1].piece = 1;
  board[2][2].piece = 1;
  board[2][3].piece = 1;
  board[2][4].piece = 1;
  board[4][0].piece = 2;
  board[3][1].piece = 2;
  board[3][3].piece = 2;
  board[3][4].piece = 2;
  board[4][2].piece = 2;
}

void help(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont, GameState *currentState, bool *running)
{
  struct text texts[3] = {0};
  struct tile board[5][5] = {0};
  int page = 0;
  texts[0] = createText(0, 20, 20, "Back", al_map_rgb(0, 0, 0), HOVER, *font);
  texts[1] = createText(1, 20, SCREEN_HEIGHT - al_get_font_line_height(*font), "Previous Tip", al_map_rgb(0, 0, 0), HOVER, *font);
  texts[2] = createText(2, SCREEN_WIDTH - al_get_text_width(*font, "Next Tip "), SCREEN_HEIGHT - al_get_font_line_height(*font), "Next Tip", al_map_rgb(0, 0, 0), HOVER, *font);

  bool helpRunning = true;

  initializeTiles(board);

  while (helpRunning)
  {
    al_clear_to_color(al_map_rgb(200, 255, 255));
    handleHelpEvents(texts, running, &helpRunning, display, font, &page, currentState);
    drawBoard(board, 0);
    drawText(texts, sizeof(texts) / sizeof(texts[0]));

    switch (page)
    {
    case 0:
      page0(board, font);
      break;
    case 1:
      page1(board, font);
      break;
    case 2:
      page2(board, font, smallFont);
      break;
    case 3:
      page3(board, font);
      break;
    case 4:
      page4(board, font, smallFont);
      break;
    case 5:
      page5(board, font, smallFont);
      break;
    case 6:
      page6(board, font);
      break;
    case 7:
      page7(board, font);
      break;
    case 8:
      page8(board, font);
      break;
    case 9:
      page9(board, font);
      break;
    case 10:
      page10(board, font);
      break;
    case 11:
      page11(board, font);
      break;
    default:
      break;
    }
    al_flip_display();
  }
}

void drawHistoryGames(Game *games, int gameCount, ALLEGRO_FONT **font, bool isComputer, int longestGame, int shortestGame, int scrollOffset)
{
  char longest[100];
  sprintf(longest, "Game %d: %.f Seconds", longestGame + 1, games[longestGame].time);
  char shortest[100];
  sprintf(shortest, "Game %d: %.f Seconds", shortestGame + 1, games[shortestGame].time);

  int x = isComputer ? SCREEN_WIDTH / 2 - (al_get_text_width(*font, "Player vs Computer") * 1.3) : SCREEN_WIDTH / 2 + (al_get_text_width(*font, "Player vs Player") * 0.3);
  int y = 80 - scrollOffset;

  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, "Games History"), y - 60, 0, "Games History");

  al_draw_text(*font, isComputer ? al_map_rgb(150, 10, 100) : al_map_rgb(0, 150, 150), x, y, 0, isComputer ? "Player vs Computer" : "Player vs Player");

  y += al_get_font_line_height(*font);

  al_draw_textf(*font, isComputer ? al_map_rgb(150, 10, 100) : al_map_rgb(0, 150, 150), x, y, 0, "Total Games: %d", gameCount);

  y += 2 * al_get_font_line_height(*font);

  al_draw_text(*font, al_map_rgb(0, 0, 0), x, y, 0, "Longest Game:");
  y += al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), x, y, 0, longest);

  y += 2 * al_get_font_line_height(*font);

  al_draw_text(*font, al_map_rgb(0, 0, 0), x, y, 0, "Shortest Game:");
  y += al_get_font_line_height(*font);
  al_draw_text(*font, al_map_rgb(0, 0, 0), x, y, 0, shortest);

  y += 2 * al_get_font_line_height(*font);

  for (int i = gameCount - 1; i >= 0; i--)
  {
    al_draw_filled_rectangle(x - 10, y - 10, x + (al_get_text_width(*font, "Time: 100 Seconds")), y + (al_get_font_line_height(*font) * 5), isComputer ? al_map_rgb(255, 200, 255) : al_map_rgb(200, 255, 255));

    al_draw_textf(*font, al_map_rgb(0, 0, 0), x, y, 0, "Game %d", i + 1);

    y += al_get_font_line_height(*font);

    al_draw_textf(*font, al_map_rgb(0, 0, 0), x, y, 0, "Turns: %d", games[i].turns);

    y += al_get_font_line_height(*font);

    al_draw_textf(*font, al_map_rgb(0, 0, 0), x, y, 0, "Time: %.f Seconds", games[i].time);

    y += al_get_font_line_height(*font);

    al_draw_textf(*font, games[i].winner == 1 ? al_map_rgb(200, 0, 0) : games[i].winner == 2 ? al_map_rgb(0, 0, 200)
                                                                                             : al_map_rgb(150, 0, 150),
                  x, y, 0, "Winner: %s", games[i].winner == 1 ? "Player 1" : games[i].winner == 2 ? "Player 2"
                                                                                                  : "None");

    y += al_get_font_line_height(*font);

    al_draw_textf(*font, al_map_rgb(0, 0, 0), x, y, 0, "Result: %s", games[i].result == 1 ? "Win" : games[i].result == 2 ? "Small Win"
                                                                                                                         : "Draw");

    y += al_get_font_line_height(*font);
    y += al_get_font_line_height(*font);
  }
}

int handleHistoryEvents(struct text texts[], ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, bool *running, GameState *currentState, int *scrollOffset, int maxScroll)
{
  ALLEGRO_EVENT_QUEUE *event_queue = setupEventQueue(display);
  ALLEGRO_EVENT ev;
  bool hoveringText = false;
  al_wait_for_event(event_queue, &ev);
  if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
  {
    al_destroy_display(*display);
  }
  else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
  {
    int mouse_x = ev.mouse.x;
    int mouse_y = ev.mouse.y;
    *scrollOffset -= ev.mouse.dz * 10;
    hoveringText = isHoveringText(texts, mouse_x, mouse_y, 1);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      *running = false;
      *currentState = MAIN_MENU;
    }
    if (*scrollOffset < 0)
    {
      *scrollOffset = 0;
    }
    else if (*scrollOffset > maxScroll)
    {
      *scrollOffset = maxScroll;
    }
  }
  return -1;
}

void history(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, GameState *currentState, Game *games, int gameCount)
{
  bool historyRunning = true;
  struct text texts[1] = {0};
  texts[0] = createText(0, 20, 20, "Back", al_map_rgb(0, 0, 0), HOVER, *font);

  Game *computerGames = malloc(gameCount * sizeof(Game));
  Game *playerGames = malloc(gameCount * sizeof(Game));
  int computers = 0;
  int players = 0;
  int longestPlayerGame = 0;
  int longestComputerGame = 0;
  int shortestPlayerGame = 0;
  int shortestComputerGame = 0;

  for (size_t i = 0; i < gameCount; i++)
  {
    if (games[i].isComputer)
    {
      computerGames[computers] = games[i];
      if (computers == 0 || games[i].time > computerGames[longestComputerGame].time)
      {
        longestComputerGame = computers;
      }
      if (computers == 0 || games[i].time < computerGames[shortestComputerGame].time)
      {
        shortestComputerGame = computers;
      }
      computers++;
    }
    else
    {
      playerGames[players] = games[i];
      if (players == 0 || games[i].time > playerGames[longestPlayerGame].time)
      {
        longestPlayerGame = players;
      }
      if (players == 0 || games[i].time < playerGames[shortestPlayerGame].time)
      {
        shortestPlayerGame = players;
      }
      players++;
    }
  }

  computerGames = realloc(computerGames, computers * sizeof(Game));
  playerGames = realloc(playerGames, players * sizeof(Game));

  int scrollOffset = 0;
  int maxScroll = computers > players ? computers * 5 * al_get_font_line_height(*font) : players * 5 * al_get_font_line_height(*font);

  while (historyRunning)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));
    drawText(texts, sizeof(texts) / sizeof(texts[0]));

    drawHistoryGames(computerGames, computers, font, true, longestComputerGame, shortestComputerGame, scrollOffset);
    drawHistoryGames(playerGames, players, font, false, longestPlayerGame, shortestPlayerGame, scrollOffset);

    handleHistoryEvents(texts, display, font, &historyRunning, currentState, &scrollOffset, maxScroll);

    al_flip_display();
  }
  free(computerGames);
  free(playerGames);
}

int main()
{
  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_FONT *font = NULL;
  ALLEGRO_FONT *smallFont = NULL;
  GameState currentState = MAIN_MENU;

  initializeAllegro(&display, &font, &smallFont);
  bool running = true;
  double savedTime = 0;
  int winner = 0;
  int gameCount = 0;
  Game *games;
  while (running)
  {
    int turn = 0;
    int pieces = 0;
    struct tile board[5][5] = {0};
    initializeTiles(board);
    al_clear_to_color(al_map_rgb(255, 255, 255));

    switch (currentState)
    {
    case MAIN_MENU:
      menu(&display, &font, &smallFont, &currentState, &running);
      break;
    case HELP:
      help(&display, &font, &smallFont, &currentState, &running);
      break;
    case GAME:
      game(&display, &font, &running, &currentState, turn, pieces, board, 0, false, &winner);
      break;
    case GAME_COMPUTER:
      game(&display, &font, &running, &currentState, turn, pieces, board, 0, true, &winner);
      break;
    case LOAD_GAME:
      loadGameState(board, &turn, "game_save.txt", &pieces, &savedTime);
      game(&display, &font, &running, &currentState, turn, pieces, board, savedTime, false, &winner);
      break;
    case LOAD_GAME_COMPUTER:
      loadGameState(board, &turn, "game_save_computer.txt", &pieces, &savedTime);
      game(&display, &font, &running, &currentState, turn, pieces, board, savedTime, true, &winner);
      break;
    case HISTORY:
      games = readGameHistory("history.txt", &gameCount);
      history(&display, &font, &currentState, games, gameCount);
      break;
    }

    al_flip_display();
    al_clear_to_color(al_map_rgb(255, 255, 255));
  }

  al_destroy_font(font);
  al_destroy_display(display);
  return 0;
}
