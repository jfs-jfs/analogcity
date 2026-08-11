

#include "random-title.h"
#include "../style.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/style/brush.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>
#include <wchar.h>

#define TITLE_HEIGHT 30
#define TITLE_WIDTH 9
static const wchar_t *TITLE[] = {
    /* 1 */ L"┏━━━━━━┓x",
    /* 2 */ L"┃ ┏━━┓ ┃x",
    /* 3 */ L"┃ ┗━━┛ ┗┓",
    /* 4 */ L"┃ ┏━━━┓ ┃",
    /* 5 */ L"┗━┛xxx┗━┛",
    /* 6 */ L"┏━━━━━━━┓",
    /* 7 */ L"┃ ┏━━━┓ ┃",
    /* 8 */ L"┃ ┗━━━┛ ┃",
    /* 9 */ L"┃ ┏━━━┓ ┃",
    /* 10 */ L"┗━┛xxx┗━┛",
    /* 11 */ L"┏━━━━━━━┓",
    /* 12 */ L"┃ ┏━━━┓ ┃",
    /* 13 */ L"┃ ┃xxx┃ ┃",
    /* 14 */ L"┃ ┃xxx┃ ┃",
    /* 15 */ L"┗━┛xxx┗━┛",
    /* 16 */ L"┏━━━━━━┓x",
    /* 17 */ L"┃ ┏━━┓ ┗┓",
    /* 18 */ L"┃ ┃xx┃  ┃",
    /* 19 */ L"┃ ┗━━┛ ┏┛",
    /* 20 */ L"┗━━━━━━┛x",
    /* 21 */ L"┏━━━━━━━┓",
    /* 22 */ L"┃ ┏━━━┓ ┃",
    /* 23 */ L"┃ ┃xxx┃ ┃",
    /* 24 */ L"┃ ┗━━━┛ ┃",
    /* 25 */ L"┗━━━━━━━┛",
    /* 26 */ L"┏━━━━━━━┓",
    /* 27 */ L"┃ ┏┓ ┏┓ ┃",
    /* 28 */ L"┃ ┃┃ ┃┃ ┃",
    /* 29 */ L"┃ ┃┗━┛┃ ┃",
    /* 30 */ L"┗━┛xxx┗━┛",
};
void random_title_render(const void *uncasted_model, struct CtCanvas *canvas) {
  struct CtBrush backup = ct_brush();

  ct_brush_fg_hex(BLUE_COLOR);
  for (size_t i = 0; i < TITLE_WIDTH + 2; i++)
    ct_cwrite_char_line(canvas, i, 0, i, canvas->max_y - 1, L'┃');

  ct_brush_fg_hex(PINK_COLOR);
  size_t starting_y = (canvas->max_y - TITLE_HEIGHT) / 2;
  for (size_t i = 0; i < TITLE_HEIGHT; i++)
    for (size_t j = 0; j < TITLE_WIDTH; j++)
      if (TITLE[i][j] != L'x')
        ct_cwrite_char(canvas, j + 1, starting_y + i, TITLE[i][j]);
  ct_brush_from(backup);
}

void random_title_setup(RandomTitle *model) {
  model->base.cleanup = _empty_cleanup;
  model->base.handler = _empty_handler;
  model->base.render = random_title_render;
}
