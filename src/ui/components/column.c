#include "column.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/brush.h>
#include <cursed-tea/ui/common.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

#define COLUMN_1_HEIGHT 14
#define COLUMN_1_WIDTH 15

static const wchar_t *COLUMN_1[] = {
    /*1*/ L"xx‗‗‗xxxxx‗‗‗xx",
    /*2*/ L"x╱ ‗ ╲═══╱ ‗ ╲x",
    /*3*/ L"( (.╲ oOo ╱.) )",
    /*4*/ L"x╲‗‗╱═════╲‗‗╱x",
    /*5*/ L"xxxx┃┃┃┃┃┃┃xxxx",
    /*6*/ L"xxxxx┃┃┃┃┃┃xxxx",
    /*7*/ L"xxxx┃┃┃┃ ┃┃xxxx",
    /*8*/ L"xxxx┃┃┃┃┃‘┃xxxx",
    /*9*/ L"xxxx┃┃ ┃┃┃┃xxxx",
    /*10*/ L"xxxx┃┃┃┃┃┃╱xxxx",
    /*11*/ L"xxxx‘┃┃┃┃┃╲xxxx",
    /*12*/ L"xxxx┃┃┃┃┃┃┃xxxx",
    /*13*/ L"xxxx╲┃┃┃┃┃┃xxxx",
    /*14*/ L"xxxx╱┃┃┃┃┃┃xxxx",
    /*15*/ L"xxxx┃┃┃┃┃┃┃xxxx",
    /*17*/ L"xxxxJ%%%%%Lxxxx",
    /*19*/ L"xxxZZZZZZZZZxxx",
};

void column_render(const void *uncasted_model, struct CtCanvas *canvas) {
  (void)uncasted_model;

  struct CtBrush backup = ct_brush();

  srand(7);
  ct_brush_add_attr(A_BOLD);
  for (size_t i = 0; i < 4; i++)
    for (size_t j = 0; j < COLUMN_1_WIDTH; j++)
      if (COLUMN_1[i][j] != L'x')
        ct_cwrite_char(canvas, j, i, COLUMN_1[i][j]);

  for (size_t i = 4; i < canvas->max_y; i++) {
    // between [4 and 15) 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
    size_t column_index = (rand() % 11) + 4;
    for (size_t j = 0; j < COLUMN_1_WIDTH; j++)
      if (COLUMN_1[column_index][j] != L'x')
        ct_cwrite_char(canvas, j, i, COLUMN_1[column_index][j]);
  }

  ct_brush_from(backup);
}

void column_handler(void *uncasted_model, const struct CtEvent *event) {
  Column *model = uncasted_model;
  if (event->type == KEY_EVENT && event->key == L' ') {
    model->counter++;
    log_fmt(LOG_DEBUG, L"counter=%zu", model->counter);
  }
}

void column_setup(Column *model) {
  model->base.cleanup = _empty_cleanup;
  // model->base.handler = _empty_handler;
  model->base.handler = column_handler;
  model->base.render = column_render;

  model->counter = 0;
}
