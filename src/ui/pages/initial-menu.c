#include "initial-menu.h"
#include "../style.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <stdbool.h>

#include "../components/events.h"

#define BANNER_HEIGT 26
#define MENU_WIDTH 33

void initial_menu_handler(void *uncasted_model, const struct CtEvent *event) {
  InitialMenu *model = uncasted_model;

  if (event->type != KEY_EVENT)
    return;

  bool fire = false;
  // Boards
  switch (event->key) {
  case L'L':
  case L'l':
    fire = true;
    model->selection = IM_LATEST;
    break;
  case L'H':
  case L'h':
    fire = true;
    model->selection = IM_HIGHWAY;
    break;
  case L'M':
  case L'm':
    fire = true;
    model->selection = IM_MEATSPACE;
    break;
  case L'T':
  case L't':
    fire = true;
    model->selection = IM_TECHNOLOGY;
    break;
  case L'E':
  case L'e':
    fire = true;
    model->selection = IM_META;
    break;
  case L'R':
  case L'r':
    fire = true;
    model->selection = IM_RANDOM;
    break;

  // OTHERS
  case L'A':
  case L'a':
    fire = true;
    model->selection = IM_ARCHIVE;
    break;
  case L'P':
  case L'p':
    fire = true;
    model->selection = IM_PRHAK;
    break;
  case L'\033': // ESC
  case L'Q':
  case L'q':
    ct_event_send_exit();
    return;
  }

  if (fire) {
    ct_event_send_custom(CUSTOM_EV_DONE, model);
  }
}

// Canvas is already the min drawing surface
void initial_menu_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const InitialMenu *model = uncasted_model;
  struct CtBrush backup = ct_brush();

  struct CtCanvas inner;

  // Title
  ct_brush_from(backup);
  ct_brush_fg_hex(PINK_COLOR);
  ct_cmargin_y(canvas, &inner, 2, 0);
  model->title.base.render(&model->title, &inner);

  // Borders
  ct_brush_border(B_DOUBLE);
  ct_cmargin_y(&inner, &inner, 15, 2);
  ct_cmargin_x(&inner, &inner, (inner.max_x - MENU_WIDTH) / 2,
               (inner.max_x - MENU_WIDTH) / 2);
  ct_cwrite_char_line(&inner, 0, 0, 32, 0, L'━');
  ct_cwrite_char_line(&inner, 0, 7, 32, 7, L'━');

  ct_brush_from(backup);

  // MENU
  ct_cwrite(&inner, 3, 0, L"[[BOARDS]]");
  ct_cwrite(&inner, 20, 0, L"[[OTHERS]]");
  ct_cmargin(&inner, &inner, 1, 0, 1, 0);

  // BOARDS == MENU
  size_t start_menu_board_x = 4;
  size_t start_menu_board_y = 0;

  // BOARDS == ACTION KEYS
  ct_brush_add_attr(A_BOLD);
  ct_brush_fg_hex(BLUE_COLOR);
  ct_cwrite_char(&inner, start_menu_board_x, start_menu_board_y, L'L');
  ct_cwrite_char(&inner, start_menu_board_x, start_menu_board_y + 1, L'H');
  ct_cwrite_char(&inner, start_menu_board_x, start_menu_board_y + 2, L'M');
  ct_cwrite_char(&inner, start_menu_board_x, start_menu_board_y + 3, L'T');
  ct_cwrite_char(&inner, start_menu_board_x + 1, start_menu_board_y + 4, L'e');
  ct_cwrite_char(&inner, start_menu_board_x, start_menu_board_y + 5, L'R');
  ct_brush_from(backup);

  // BOARDS == NAMES
  ct_cwrite(&inner, start_menu_board_x + 1, start_menu_board_y, L"atest");
  ct_cwrite(&inner, start_menu_board_x + 1, start_menu_board_y + 1, L"ighway");
  ct_cwrite(&inner, start_menu_board_x + 1, start_menu_board_y + 2,
            L"eatspace");
  ct_cwrite(&inner, start_menu_board_x + 1, start_menu_board_y + 3,
            L"echnology");
  ct_cwrite_char(&inner, start_menu_board_x, start_menu_board_y + 4, L'M');
  ct_cwrite(&inner, start_menu_board_x + 2, start_menu_board_y + 4, L"ta");
  ct_cwrite(&inner, start_menu_board_x + 1, start_menu_board_y + 5, L"andom");

  // OTHERS == MENU
  size_t start_menu_other_x = 21;
  size_t start_menu_other_y = 1;

  // OTHERS == ACTION KEYS
  ct_brush_add_attr(A_BOLD);
  ct_brush_fg_hex(BLUE_COLOR);
  ct_cwrite_char(&inner, start_menu_other_x, start_menu_other_y, L'A');
  ct_cwrite_char(&inner, start_menu_other_x, start_menu_other_y + 1, L'P');
  ct_cwrite_char(&inner, start_menu_other_x, start_menu_other_y + 3, L'Q');
  ct_brush_from(backup);

  // OTHERS == NAMES
  ct_cwrite(&inner, start_menu_other_x + 1, start_menu_other_y, L"rchive");
  ct_cwrite(&inner, start_menu_other_x + 1, start_menu_other_y + 1, L"rhak");
  ct_cwrite(&inner, start_menu_other_x + 1, start_menu_other_y + 3,
            L"uit/Exit");
  ct_brush_from(backup);
}

void initial_menu_cleanup(void *uncasted_model) {
  InitialMenu *model = uncasted_model;
  model->title.base.cleanup(&model->title);
}

void initial_menu_setup(InitialMenu *model) {
  model->base.cleanup = initial_menu_cleanup;
  model->base.render = initial_menu_render;
  model->base.handler = initial_menu_handler;

  title_setup(&model->title);
}
