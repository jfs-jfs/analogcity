#include "title.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/ui/common.h>

#define TITLE_HEIGHT 9
#define TITLE_WIDTH 6 * 9
static const wchar_t *TITLE[] = {
    /* 1 */ L"┏━━━━━━━┓┏━━━━━━━┓┏━━━━━━━┓┏━┓xxxxxx┏━━━━━━━┓┏━━━━━━━┓",
    /* 2 */ L"┃ ┏━━━┓ ┃┃ ┏━━━┓ ┃┃ ┏━━━┓ ┃┃ ┃xxxxxx┃ ┏━━━┓ ┃┃ ┏━━━━━┛",
    /* 3 */ L"┃ ┗━━━┛ ┃┃ ┃xxx┃ ┃┃ ┗━━━┛ ┃┃ ┃xxxxxx┃ ┃xxx┃ ┃┃ ┃ ┏━━━┓",
    /* 4 */ L"┃ ┏━━━┓ ┃┃ ┃xxx┃ ┃┃ ┏━━━┓ ┃┃ ┗━━━━━┓┃ ┗━━━┛ ┃┃ ┗━┛   ┃",
    /* 5 */ L"┗━┛xxx┗━┛┗━┛xxx┗━┛┗━┛xxx┗━┛┗━━━━━━━┛┗━━━━━━━┛┗━━━━━━━┛",
    /* 6 */ L"xxxxxxxxx┏━━━━━━━┓┏━━━━━━━┓┏━━━━━━━┓┏━┓xxx┏━┓xxxxxxxxx",
    /* 7 */ L"xxxxxxxxx┃ ┏━━━━━┛┗━━┓ ┏━━┛┗━━┓ ┏━━┛┃ ┗━━━┛ ┃xxxxxxxxx",
    /* 8 */ L"xxxxxxxxx┃ ┗━━━━━┓┏━━┛ ┗━━┓xxx┃ ┃xxx┗━━┓ ┏━━┛xxxxxxxxx",
    /* 9 */ L"xxxxxxxxx┗━━━━━━━┛┗━━━━━━━┛xxx┗━┛xxxxxx┗━┛xxxxxxxxxxxx",
};

void title_render(const void *uncasted_model, struct CtCanvas *canvas) {
  size_t line_lenght = TITLE_WIDTH;
  size_t center_start = (canvas->max_x - line_lenght) / 2;
  for (size_t i = 0; i < TITLE_HEIGHT; i++)
    for (size_t j = 0; j < line_lenght; j++)
      if (TITLE[i][j] != L'x')
        ct_cwrite_char(canvas, center_start + j, i, TITLE[i][j]);
}

void title_setup(Title *model) {
  model->base.cleanup = _empty_cleanup;
  model->base.handler = _empty_handler;
  model->base.render = title_render;
}
