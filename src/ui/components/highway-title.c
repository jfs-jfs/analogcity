#include "highway-title.h"
#include "../style.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/style/brush.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>
#include <wchar.h>

#define TITLE_BIG_HEIGHT 35
#define TITLE_WIDTH 9
static const wchar_t *TITLE_BIG[] = {
    /* 1 */ L"┏━┓xxx┏━┓",
    /* 2 */ L"┃ ┃xxx┃ ┃",
    /* 3 */ L"┃ ┗━━━┛ ┃",
    /* 4 */ L"┃ ┏━━━┓ ┃",
    /* 5 */ L"┗━┛xxx┗━┛",
    /* 6 */ L"┏━━━━━━━┓",
    /* 7 */ L"┗━━┓ ┏━━┛",
    /* 8 */ L"xxx┃ ┃xxx",
    /* 9 */ L"┏━━┛ ┗━━┓",
    /* 10 */ L"┗━━━━━━━┛",
    /* 11 */ L"┏━━━━━━━┓",
    /* 12 */ L"┃ ┏━━━━━┛",
    /* 13 */ L"┃ ┃ ┏━━━┓",
    /* 14 */ L"┃ ┗━┛   ┃",
    /* 15 */ L"┗━━━━━━━┛",
    /* 16 */ L"┏━┓xxx┏━┓",
    /* 17 */ L"┃ ┃xxx┃ ┃",
    /* 18 */ L"┃ ┗━━━┛ ┃",
    /* 19 */ L"┃ ┏━━━┓ ┃",
    /* 20 */ L"┗━┛xxx┗━┛",
    /* 21 */ L"┏━┓xxx┏━┓",
    /* 22 */ L"┃ ┃┏━┓┃ ┃",
    /* 23 */ L"┃ ┃┃ ┃┃ ┃",
    /* 24 */ L"┃ ┗┛ ┗┛ ┃",
    /* 25 */ L"┗━━━━━━━┛",
    /* 26 */ L"┏━━━━━━━┓",
    /* 27 */ L"┃ ┏━━━┓ ┃",
    /* 28 */ L"┃ ┗━━━┛ ┃",
    /* 29 */ L"┃ ┏━━━┓ ┃",
    /* 30 */ L"┗━┛xxx┗━┛",
    /* 31 */ L"┏━┓xxx┏━┓",
    /* 32 */ L"┃ ┃xxx┃ ┃",
    /* 33 */ L"┃ ┗━━━┛ ┃",
    /* 34 */ L"┗━━┓ ┏━━┛",
    /* 35 */ L"xxx┗━┛xxx",
};

#define TITLE_SMALL_HEIGHT 30
static const wchar_t *TITLE_SMALL[] = {
    /* 1 */ L"┏━┓xxx┏━┓",
    /* 2 */ L"┃ ┗━━━┛ ┃",
    /* 3 */ L"┃ ┏━━━┓ ┃",
    /* 4 */ L"┗━┛xxx┗━┛",
    /* 5 */ L"┏━━━━━━━┓",
    /* 6 */ L"┗━━┓ ┏━━┛",
    /* 7 */ L"┏━━┛ ┗━━┓",
    /* 8 */ L"┗━━━━━━━┛",
    /* 9 */ L"┏━━━━━━━┓",
    /* 10 */ L"┃ ┏━━━━━┛",
    /* 11 */ L"┃ ┃ ┏━━━┓",
    /* 12 */ L"┃ ┗━┛   ┃",
    /* 13 */ L"┗━━━━━━━┛",
    /* 14 */ L"┏━┓xxx┏━┓",
    /* 15 */ L"┃ ┗━━━┛ ┃",
    /* 16 */ L"┃ ┏━━━┓ ┃",
    /* 17 */ L"┗━┛xxx┗━┛",
    /* 18 */ L"┏━┓xxx┏━┓",
    /* 19 */ L"┃ ┃┏━┓┃ ┃",
    /* 20 */ L"┃ ┗┛ ┗┛ ┃",
    /* 21 */ L"┗━━━━━━━┛",
    /* 22 */ L"┏━━━━━━━┓",
    /* 23 */ L"┃ ┏━━━┓ ┃",
    /* 24 */ L"┃ ┗━━━┛ ┃",
    /* 25 */ L"┃ ┏━━━┓ ┃",
    /* 26 */ L"┗━┛xxx┗━┛",
    /* 27 */ L"┏━┓xxx┏━┓",
    /* 28 */ L"┃ ┗━━━┛ ┃",
    /* 29 */ L"┗━━┓ ┏━━┛",
    /* 30 */ L"xxx┗━┛xxx",
};

void hightway_title_render(const void *uncasted_model,
                           struct CtCanvas *canvas) {
  struct CtBrush backup = ct_brush();

  ct_brush_fg_hex(BLUE_COLOR);
  for (size_t i = 0; i < TITLE_WIDTH + 2; i++)
    ct_cwrite_char_line(canvas, i, 0, i, canvas->max_y - 1, L'┃');

  ct_brush_fg_hex(PINK_COLOR);
  if (canvas->max_y >= TITLE_BIG_HEIGHT) {
    size_t starting_y = (canvas->max_y - TITLE_BIG_HEIGHT) / 2;
    for (size_t i = 0; i < TITLE_BIG_HEIGHT; i++)
      for (size_t j = 0; j < TITLE_WIDTH; j++)
        if (TITLE_BIG[i][j] != L'x')
          ct_cwrite_char(canvas, j + 1, starting_y + i, TITLE_BIG[i][j]);
  } else {
    size_t starting_y = (canvas->max_y - TITLE_SMALL_HEIGHT) / 2;
    for (size_t i = 0; i < TITLE_SMALL_HEIGHT; i++)
      for (size_t j = 0; j < TITLE_WIDTH; j++)
        if (TITLE_SMALL[i][j] != L'x')
          ct_cwrite_char(canvas, j + 1, starting_y + i, TITLE_SMALL[i][j]);
  }
  ct_brush_from(backup);
}

void highway_title_setup(HighwayTitle *model) {
  model->base.cleanup = _empty_cleanup;
  model->base.handler = _empty_handler;
  model->base.render = hightway_title_render;
}
