#include "archive-title.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>
#include <wchar.h>

#define TITLE_HEIGHT 5
#define TITLE_LENGHT 63
const static wchar_t *TITLE[] = {
    /* 1 */ L"┏━━━━━━━┓┏━━━━━━┓x┏━━━━━━━┓┏━┓xxx┏━┓┏━━━━━━━┓┏━┓xxx┏━┓┏━━━━━━━┓",
    /* 2 */ L"┃ ┏━━━┓ ┃┃ ┏━━┓ ┃x┃ ┏━━━━━┛┃ ┃xxx┃ ┃┗━━┓ ┏━━┛┃ ┃xxx┃ ┃┃ ┏━━━━━┛",
    /* 3 */ L"┃ ┗━━━┛ ┃┃ ┗━━┛ ┗┓┃ ┃xxxxxx┃ ┗━━━┛ ┃xxx┃ ┃xxx┃ ┗┓x┏┛ ┃┃ ┗━━━┓xx",
    /* 4 */ L"┃ ┏━━━┓ ┃┃ ┏━━━┓ ┃┃ ┗━━━━━┓┃ ┏━━━┓ ┃┏━━┛ ┗━━┓┗┓ ┗━┛ ┏┛┃  ━━━┻━┓",
    /* 5 */ L"┗━┛xxx┗━┛┗━┛xxx┗━┛┗━━━━━━━┛┗━┛xxx┗━┛┗━━━━━━━┛x┗━━━━━┛x┗━━━━━━━┛",
};

void archive_title_render(const void *uncasted_model, struct CtCanvas *canvas) {
  (void)uncasted_model;

  if (canvas->max_x < TITLE_LENGHT) {
    log_error(L"Canvas to small (width) for archive title");
    return;
  } else if (canvas->max_y < TITLE_HEIGHT) {
    log_error(L"Canvas to small (height) for archive title");
    return;
  }

  size_t starting_x = (canvas->max_x - TITLE_LENGHT) / 2;
  size_t starting_y = (canvas->max_y - TITLE_HEIGHT) / 2;

  for (size_t i = 0; i < TITLE_HEIGHT; i++)
    for (size_t j = 0; j < TITLE_LENGHT; j++)
      if (TITLE[i][j] != L'x')
        ct_cwrite_char(canvas, starting_x + j, starting_y + i, TITLE[i][j]);
}

void archive_title_setup(ArchiveTitle *model) {
  model->base.cleanup = _empty_cleanup;
  model->base.handler = _empty_handler;
  model->base.render = archive_title_render;
}
