#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define TILE_SIZE 50
#define CENTER_BOARD_X (SCREEN_WIDTH / 2) - (TILE_SIZE * 5 / 2)
#define CENTER_BOARD_Y (SCREEN_HEIGHT / 2) - (TILE_SIZE * 5 / 2)
#define TILE_END_X CENTER_BOARD_X + TILE_SIZE
#define TILE_END_Y CENTER_BOARD_Y + TILE_SIZE
#define TILE1_COLOR al_map_rgb(0, 0, 0)
#define TILE2_COLOR al_map_rgb(100, 100, 100)
#define TILE3_COLOR al_map_rgb(255, 122, 255)
#define HOVER al_map_rgb(255, 0, 0)
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
  ALLEGRO_COLOR colorHover;
  bool hover;
  bool click;
  bool positionable;
  int piece;
};

struct tile createTile(int id, int x, int y, int x1, int y1, int x2, int y2, ALLEGRO_COLOR color, ALLEGRO_COLOR colorHover)
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
  t.colorHover = colorHover;
  t.hover = false;
  t.click = false;
  t.positionable = false;
  t.piece = 0;
  return t;
}

ALLEGRO_EVENT_QUEUE *setupEventQueue(ALLEGRO_DISPLAY **display)
{
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(*display));
  al_register_event_source(event_queue, al_get_mouse_event_source());
  return event_queue;
}

void drawTile(struct tile tile)
{
  if (tile.hover)
  {
    al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, tile.colorHover);
  }
  else if (tile.positionable)
  {
    al_draw_filled_rectangle(tile.x1, tile.y1, tile.x2, tile.y2, al_map_rgba(0, 255, 0, 100));
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
        al_draw_filled_rectangle((CENTER_BOARD_X - TILE_SIZE) + x * TILE_SIZE, (CENTER_BOARD_Y - TILE_SIZE) + y * TILE_SIZE, (TILE_END_X - TILE_SIZE) + x * TILE_SIZE, (TILE_END_Y - TILE_SIZE) + y * TILE_SIZE, al_map_rgb(150, 100, 50));
      }
      else if (x > 0 && y > 0 && x < 6 && y < 6)
      {
        drawTile(tile[x - 1][y - 1]);
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

void initializeTiles(struct tile tile[5][5])
{
  int id = 0;
  for (size_t x = 0; x < 5; x++)
  {
    for (size_t y = 0; y < 5; y++)
    {
      if (x == 2 && y == 2)
      {
        tile[x][y] = createTile(id, x, y, CENTER_BOARD_X + x * TILE_SIZE, CENTER_BOARD_Y + y * TILE_SIZE, TILE_END_X + x * TILE_SIZE, TILE_END_Y + y * TILE_SIZE, TILE3_COLOR, HOVER);
      }
      else if ((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0))
      {
        tile[x][y] = createTile(id, x, y, CENTER_BOARD_X + x * TILE_SIZE, CENTER_BOARD_Y + y * TILE_SIZE, TILE_END_X + x * TILE_SIZE, TILE_END_Y + y * TILE_SIZE, TILE1_COLOR, HOVER);
      }
      else
      {
        tile[x][y] = createTile(id, x, y, CENTER_BOARD_X + x * TILE_SIZE, CENTER_BOARD_Y + y * TILE_SIZE, TILE_END_X + x * TILE_SIZE, TILE_END_Y + y * TILE_SIZE, TILE2_COLOR, HOVER);
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

void initializeMenuTexts(struct text texts[6], ALLEGRO_FONT *font)
{
  texts[0] = createText(0, getCenter(font, "Player V.S Player"), 100, "Player V.S Player", al_map_rgb(0, 0, 0), HOVER, font);
  texts[1] = createText(1, getCenter(font, "Player V.S Computer"), 150, "Player V.S Computer", al_map_rgb(0, 0, 0), HOVER, font);
  texts[2] = createText(2, getCenter(font, "Continue Last Game"), 200, "Continue Last Game", al_map_rgb(0, 0, 0), HOVER, font);
  texts[3] = createText(3, getCenter(font, "History"), 250, "History", al_map_rgb(0, 0, 0), HOVER, font);
  texts[4] = createText(4, getCenter(font, "Help"), 300, "Help", al_map_rgb(0, 0, 0), HOVER, font);
  texts[5] = createText(5, getCenter(font, "Quit"), 350, "Quit", al_map_rgb(0, 0, 0), HOVER, font);
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

  *font = al_load_ttf_font("MinecraftRegular-Bmg3.otf", 32, 0);
  if (!*font)
  {
    fprintf(stderr, "Failed to load font. Make sure 'MinecraftRegular-Bmg3.otf' is in the correct directory.\n");
    al_destroy_display(*display);
    return false;
  }

  *smallFont = al_load_ttf_font("MinecraftRegular-Bmg3.otf", 24, 0);
  if (!*smallFont)
  {
    fprintf(stderr, "Failed to load font. Make sure 'MinecraftRegular-Bmg3.otf' is in the correct directory.\n");
    al_destroy_display(*display);
    return false;
  }

  return true;
}

void startGame(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font);

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

void setTilePossibility(struct tile board[5][5], int i, int j, bool hasPossibilities)
{
  bool tileUp, tileDown, tileLeft, tileRight;
  clearPositionable(board);
  if (checkTilePossibilities(board, i, j, &tileUp, &tileDown, &tileLeft, &tileRight) && hasPossibilities)
  {
    if (j - 1 >= 0)
    {
      board[i][j - 1].positionable = tileUp;
    }
    if (j + 1 < 5)
    {
      board[i][j + 1].positionable = tileDown;
    }
    if (i - 1 >= 0)
    {
      board[i - 1][j].positionable = tileLeft;
    }
    if (i + 1 < 5)
    {
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
    printf("No possibilities\n");
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

void checkEat(struct tile board[5][5], int x, int y, int turn)
{
  if (turn % 2 != 0)
  {
    if (x + 2 < 5)
    {
      if (board[x + 1][y].piece == 1 && board[x + 1][y].id != 12 && board[x + 2][y].piece == 2)
      {
        board[x + 1][y].piece = 0;
      }
    }
    if (x - 2 >= 0)
    {
      if (board[x - 1][y].piece == 1 && board[x - 1][y].id != 12 && board[x - 2][y].piece == 2)
      {
        board[x - 1][y].piece = 0;
      }
    }
    if (y + 2 < 5)
    {
      if (board[x][y + 1].piece == 1 && board[x][y + 1].id != 12 && board[x][y + 2].piece == 2)
      {
        board[x][y + 1].piece = 0;
      }
    }
    if (y - 2 >= 0)
    {
      if (board[x][y - 1].piece == 1 && board[x][y - 1].id != 12 && board[x][y - 2].piece == 2)
      {
        board[x][y - 1].piece = 0;
      }
    }
  }
  else
  {
    if (x + 2 < 5)
    {
      if (board[x + 1][y].piece == 2 && board[x + 1][y].id != 12 && board[x + 2][y].piece == 1)
      {
        board[x + 1][y].piece = 0;
      }
    }
    if (x - 2 >= 0)
    {
      if (board[x - 1][y].piece == 2 && board[x - 1][y].id != 12 && board[x - 2][y].piece == 1)
      {
        board[x - 1][y].piece = 0;
      }
    }
    if (y + 2 < 5)
    {
      if (board[x][y + 1].piece == 2 && board[x][y + 1].id != 12 && board[x][y + 2].piece == 1)
      {
        board[x][y + 1].piece = 0;
      }
    }
    if (y - 2 >= 0)
    {
      if (board[x][y - 1].piece == 2 && board[x][y - 1].id != 12 && board[x][y - 2].piece == 1)
      {
        board[x][y - 1].piece = 0;
      }
    }
  }
}

void checkSmallWin(struct tile board[5][5], int x, int y, bool *running)
{
  int winX = 0;
  int winY = 0;
  int piece = board[x][y].piece;
  for (size_t i = 0; i < 5; i++)
  {
    if (board[x][i].piece == piece)
    {
      winY++;
    }
    if (winY == 5)
    {
      *running = false;
      printf("SmallWinX");
    }
  }
  for (size_t i = 0; i < 5; i++)
  {
    if (board[i][y].piece == piece)
    {
      winX++;
    }
    if (winX == 5)
    {
      *running = false;
      printf("SmallWinY");
    }
  }
}

void checkWin(struct tile board[5][5], int turn, bool *running)
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
  if (player1 <= 3)
  {
    printf("Player 2 wins\n");
    *running = false;
  }
  else if (player2 <= 3)
  {
    printf("Player 1 wins\n");
    *running = false;
  }
}

void movePiece(struct tile board[5][5], int x, int y, struct tile *selectedTile)
{
  board[x][y].piece = selectedTile->piece;
  selectedTile->id = -1;
  board[selectedTile->x][selectedTile->y].piece = 0;
  clearPositionable(board);
}

bool onMouseClickTile(ALLEGRO_DISPLAY **display, struct tile board[5][5], int mouse_x, int mouse_y, bool *running, int *turn, struct tile *selectedTile, bool hasPossibilities)
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
        else if (board[i][j].piece == 1 && *turn >= 12 && *turn % 2 == 0)
        {
          setTilePossibility(board, i, j, hasPossibilities);
          *selectedTile = board[i][j];
        }
        else if (board[i][j].piece == 2 && *turn >= 12 && *turn % 2 != 0)
        {
          setTilePossibility(board, i, j, hasPossibilities);
          *selectedTile = board[i][j];
        }
        else if (board[i][j].positionable && selectedTile->id != -1)
        {
          movePiece(board, i, j, selectedTile);
          checkEat(board, i, j, *turn);
          checkWin(board, *turn, running);
          checkSmallWin(board, i, j, running);
          return true;
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

void onMouseClickHelpText(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, struct text texts[], int mouse_x, int mouse_y, bool *running, int *page)
{
  switch (mouseClickText(texts, mouse_x, mouse_y))
  {
  case 0:
    *running = false;
    break;
  case 1:
    *page = *page - 1;
    break;
  case 2:
    *page = *page + 1;
    break;
  default:
    break;
  }
}

int handleHelpEvents(struct text texts[], bool *running, ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, int *page)
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
    isHoveringText(texts, mouse_x, mouse_y, &hoveringText);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      onMouseClickHelpText(display, font, texts, mouse_x, mouse_y, running, page);
      return 0;
    }
    al_destroy_event_queue(event_queue);
  }
  return -1;
}

void page0(struct tile board[5][5], ALLEGRO_FONT **font)
{
  /*
  1. The game is played on a 5x5 board.
  2. Each player has 12 pieces, player 1 is red, and player 2 is blue.
  3. The game has two phases: placement and movement.
  4. In the placement phase, players take turns placing 2 of their pieces on the board each turn, players CANNOT place their pieces in the middle during this phase.
  5. In the movement phase, players take turns moving their pieces to adjacent empty spaces.
  6. The objective is to eat all the opponent's pieces.
  7. You can eat an opponent's piece by surrounding it with your pieces in your turn.
  8. You can only eat a piece when its your turn.
  9. A piece can only move to an adjacent empty space.
  10. You can eat an opponent's piece if stuck with no empty spaces to move.
  11. You can't eat the opponent's piece if it's in the center of the board.
  12. You win by eating all the opponent's pieces.
  13. You can have a small win by aligning all your pieces in a row or column, separating the board in two, with all the opponents pieces on one side.
  14. The game draws if both players have 3 or less pieces.
  */
  initializeTiles(board);
  char text[100] = "1. The game is played on a 5x5 board.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
}

void page1(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "2. Each player has 12 pieces,";
  char text1[100] = "player 1 is red, and player 2 is blue.";
  board[1][2].piece = 1;
  board[3][2].piece = 2;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 70, 0, text1);
}

void page2(struct tile board[5][5], ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  initializeTiles(board);
  char text[100] = "3. Placement phase:";
  char text1[100] = "Players take turns placing 2 of their pieces on the board each turn.";
  char text2[100] = "players CANNOT place their pieces in the middle during this phase.";
  board[2][2].color = al_map_rgb(255, 0, 0);
  board[4][2].piece = 1;
  board[3][1].piece = 2;
  board[1][2].piece = 1;
  board[3][2].piece = 2;
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text1), 65, 0, text1);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text2), 88, 0, text2);
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
  char text[100] = "4. Movement phase:";
  char text1[100] = "Players take turns moving their pieces to adjacent empty spaces.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 70, 0, text1);
  adjacentSpaces(board);
}

void page4(struct tile board[5][5], ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  initializeTiles(board);
  char text[100] = "5. Objective:";
  char text1[100] = "The objective is to eat all the opponent's pieces.";
  char text2[100] = "You can eat an opponent's piece by surrounding it with your pieces in your turn.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 60, 0, text1);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text2), 90, 0, text2);
  adjacentSpaces(board);
}

void page5(struct tile board[5][5], ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  initializeTiles(board);
  char text[100] = "5. Objective:";
  char text1[100] = "The objective is to eat all the opponent's pieces.";
  char text2[100] = "You can eat an opponent's piece by surrounding it with your pieces in your turn.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 60, 0, text1);
  al_draw_text(*smallFont, al_map_rgb(0, 0, 0), getCenter(*smallFont, text2), 90, 0, text2);
  board[4][2].piece = 1;
  board[3][1].piece = 2;
  board[2][2].piece = 1;
}

void page6(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "You CANNOT eat an opponents piece";
  char text1[100] = "if it's in the center of the board.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 60, 0, text1);
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
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 60, 0, text1);
  stuckPieces(board);
}

void page8(struct tile board[5][5], ALLEGRO_FONT **font)
{
  initializeTiles(board);
  char text[100] = "If stuck with no empty spaces to move.";
  char text1[100] = "You can eat an opponent's piece.";
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 60, 0, text1);
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
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
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
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 30, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 60, 0, text1);
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
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text), 55, 0, text);
  al_draw_text(*font, al_map_rgb(0, 0, 0), getCenter(*font, text1), 85, 0, text1);
  al_draw_text(*font, al_map_rgb(200, 0, 0), getCenter(*font, text2), SCREEN_HEIGHT - al_get_font_line_height(*font), 0, text2);
  board[3][2].piece = 2;
  board[1][4].piece = 2;
  board[1][3].piece = 2;
  board[2][0].piece = 1;
  board[0][4].piece = 1;
  board[4][3].piece = 1;
}

void helpView(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  bool running = true;
  struct text texts[3] = {0};
  struct tile board[5][5] = {0};
  int page = 0;
  texts[0] = createText(0, 20, 20, "Back", al_map_rgb(0, 0, 0), HOVER, *font);
  texts[1] = createText(1, 20, SCREEN_HEIGHT - al_get_font_line_height(*font), "Previous Tip", al_map_rgb(0, 0, 0), HOVER, *font);
  texts[2] = createText(2, SCREEN_WIDTH - al_get_text_width(*font, "Next Tip "), SCREEN_HEIGHT - al_get_font_line_height(*font), "Next Tip", al_map_rgb(0, 0, 0), HOVER, *font);

  initializeTiles(board);

  while (running)
  {
    al_clear_to_color(al_map_rgb(200, 255, 255));
    handleHelpEvents(texts, &running, display, font, &page);
    drawBoard(board);
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

void onMouseClickMenuText(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont, struct text texts[], int mouse_x, int mouse_y, bool *running)
{
  switch (mouseClickText(texts, mouse_x, mouse_y))
  {
  case 0:
    startGame(display, font);
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
    helpView(display, font, smallFont);
    break;
  case 5:
    *running = false;
    break;
  default:
    break;
  }
}

int handleGameEvents(struct text texts[], bool *running, ALLEGRO_DISPLAY **display, struct tile board[5][5], ALLEGRO_FONT **font, int *round, struct tile *selectedTile, bool hasPossibilities)
{
  bool hoveringTile = false;
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

    isHoveringTile(board, mouse_x, mouse_y, &hoveringTile);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringTile)
    {
      if (onMouseClickTile(display, board, mouse_x, mouse_y, running, round, selectedTile, hasPossibilities))
      {
        return 1;
      }
    }
  }
  return -1;
}

void startGame(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font)
{
  bool running = true;
  struct text texts[6] = {0};
  struct tile selectedTile = {0};
  int turn = 0;
  int pieces = 0;
  bool hasPossibilities = false;
  struct tile board[5][5] = {0};
  initializeTiles(board);
  while (running)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));

    drawBoard(board);

    al_draw_text(*font, al_map_rgb(0, 0, 0), 10, 10, 0, "Turn: ");
    al_draw_textf(*font, turn % 2 == 0 ? al_map_rgb(200, 0, 0) : al_map_rgb(0, 0, 200), 100, 10, 0, "%s", turn % 2 == 0 ? "Player 1" : "Player 2");

    al_flip_display();

    hasPossibilities = checkPossibilities(board, turn);

    if (handleGameEvents(texts, &running, display, board, font, &turn, &selectedTile, hasPossibilities) == 1)
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
  }
}

int handleMenuEvents(struct text texts[], bool *running, ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
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
    isHoveringText(texts, mouse_x, mouse_y, &hoveringText);
    if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && hoveringText)
    {
      onMouseClickMenuText(display, font, smallFont, texts, mouse_x, mouse_y, running);
      return 0;
    }
    al_destroy_event_queue(event_queue);
  }
  return -1;
}

void menu(ALLEGRO_DISPLAY **display, ALLEGRO_FONT **font, ALLEGRO_FONT **smallFont)
{
  bool running = true;
  struct text texts[6] = {0};

  initializeMenuTexts(texts, *font);

  while (running)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));

    drawText(texts, sizeof(texts) / sizeof(texts[0]));

    al_flip_display();

    handleMenuEvents(texts, &running, display, font, smallFont);
  }

  al_destroy_font(*font);
  al_destroy_display(*display);
}

int main()
{
  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_FONT *font = NULL;
  ALLEGRO_FONT *smallFont = NULL;
  initializeAllegro(&display, &font, &smallFont);
  menu(&display, &font, &smallFont);
  return 0;
}
