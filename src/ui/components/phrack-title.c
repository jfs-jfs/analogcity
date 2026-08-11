
#include "phrack-title.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>
#include <wchar.h>

#define TITLE_HEIGHT 5
#define TITLE_LENGHT 45
const static wchar_t *TITLE[] = {
    /* 1 */ L"┏━━━━━━━┓┏━┓xxx┏━┓┏━━━━━━┓x┏━━━━━━━┓┏━┓xx┏━┓x",
    /* 2 */ L"┃ ┏━━━┓ ┃┃ ┃xxx┃ ┃┃ ┏━━┓ ┃x┃ ┏━━━┓ ┃┃ ┃xx┃ ┃x",
    /* 3 */ L"┃ ┗━━━┛ ┃┃ ┗━━━┛ ┃┃ ┗━━┛ ┗┓┃ ┗━━━┛ ┃┃ ┗━━┛ ┗┓",
    /* 4 */ L"┃ ┏━━━━━┛┃ ┏━━━┓ ┃┃ ┏━━━┓ ┃┃ ┏━━━┓ ┃┃ ┏━━━┓ ┃",
    /* 5 */ L"┗━┛xxxxxx┗━┛xxx┗━┛┗━┛xxx┗━┛┗━┛xxx┗━┛┗━┛xxx┗━┛",
};

void phrack_title_render(const void *uncasted_model, struct CtCanvas *canvas) {
  (void)uncasted_model;

  if (canvas->max_x < TITLE_LENGHT) {
    log_error(L"Canvas to small (width) for phrack title");
    return;
  } else if (canvas->max_y < TITLE_HEIGHT) {
    log_error(L"Canvas to small (height) for phrack title");
    return;
  }

  size_t starting_x = (canvas->max_x - TITLE_LENGHT) / 2;
  size_t starting_y = (canvas->max_y - TITLE_HEIGHT) / 2;

  for (size_t i = 0; i < TITLE_HEIGHT; i++)
    for (size_t j = 0; j < TITLE_LENGHT; j++)
      if (TITLE[i][j] != L'x')
        ct_cwrite_char(canvas, starting_x + j, starting_y + i, TITLE[i][j]);
}

void phrack_title_setup(PhrackTitle *model) {
  model->base.cleanup = _empty_cleanup;
  model->base.handler = _empty_handler;
  model->base.render = phrack_title_render;
}
