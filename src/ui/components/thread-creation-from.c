#include "thread-creation-from.h"
#include "../../file_operations.h"
#include "../../helpers.h"
#include "../style.h"
#include "events.h"
#include "label-input.h"
#include "text-area-input.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/helpers.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/style/border.h>
#include <cursed-tea/style/brush.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <wchar.h>

void *create_thread_file_in_background(void *uncasted_model) {

  ThreadCreationFrom *model = uncasted_model;
  char board_name[BOARD_MAX_NAME + 1];
  to_board_name(model->for_board, board_name);
  board_name[BOARD_MAX_NAME] = '\0';
  create_thread_file(board_name, model->title_input._buffer,
                     model->author_input._buffer, model->op_input._buffer);
  model->created_thread = true;
  ct_event_send_custom_delayed(CUSTOM_EV_DONE, 10, uncasted_model);
  return NULL;
}

void thread_creation_cleanup(void *uncasted_model) {

  ThreadCreationFrom *model = uncasted_model;

  ct_subcleanup(&model->title_input.base);
  ct_subcleanup(&model->op_input.base);
}

void thread_creation_handler(void *uncasted_model,
                             const struct CtEvent *event) {
  ThreadCreationFrom *model = uncasted_model;

  if (model->_disable_input)
    return;

  if (event->type == KEY_EVENT) {
    switch (event->key) {
    case L'\033':
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case L'\t':
      if (model->author_input.focus) {
        label_input_unfocus(&model->author_input);
        label_input_focus(&model->title_input);
      } else if (model->title_input.focus) {
        label_input_unfocus(&model->title_input);
        textarea_focus(&model->op_input);
      } else {
        textarea_unfocus(&model->op_input);
        label_input_focus(&model->author_input);
      }
      break;
    case KEY_F(9):
      // Fire without creating the file
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case KEY_F(10):
      if (wcslen(model->title_input._buffer) <= THREAD_MIN_TITLE) {
        model->_error_title = true;
        model->_error_op = false;
        model->_error_author = false;
        return;
      }
      if (wcslen(model->op_input._buffer) <= REPLY_MIN_SIZE) {
        model->_error_title = false;
        model->_error_op = true;
        model->_error_author = false;
        return;
      }
      if (wcslen(model->author_input._buffer) <= AUTHOR_MIN_NAME) {
        model->_error_title = false;
        model->_error_op = false;
        model->_error_author = true;
        return;
      }
      model->_disable_input = true;
      pthread_t create_file_thread;
      pthread_create(&create_file_thread, NULL,
                     create_thread_file_in_background, model);
      pthread_detach(create_file_thread);
      break;
    }
  }

  model->title_input.base.handler(&model->title_input, event);
  model->author_input.base.handler(&model->author_input, event);
  model->op_input.base.handler(&model->op_input, event);
}
void thread_creation_render(const void *uncasted_model,
                            struct CtCanvas *canvas) {

  const ThreadCreationFrom *model = uncasted_model;

  struct CtBrush backup = ct_brush();
  struct CtCanvas surface, inner_surface, text_area;
  ct_ccutout_c(canvas, &surface, canvas->max_x - 2, 24);
  ct_cfill(&surface, L' ');

  ct_brush_fg_hex(PINK_COLOR);
  ct_cborder(&surface, &inner_surface);

  ct_brush_from(backup);
  ct_brush_attr(A_BOLD);
  ct_cwrite(&surface, 3, 0, L"[NEW THREAD FORM]");

  if (model->_error_title) {
    ct_brush_fg_hex(RED_COLOR);
    ct_cwrite(&surface, 21, 0, L"[!TITLE IS TOO SHORT!]");
    ct_brush_from(backup);
  } else if (model->_error_op) {
    ct_brush_fg_hex(RED_COLOR);
    ct_cwrite(&surface, 21, 0, L"[!OP TEXT IS TOO SHORT!]");
    ct_brush_from(backup);
  } else if (model->_error_author) {
    ct_brush_fg_hex(RED_COLOR);
    ct_cwrite(&surface, 21, 0, L"[!AUTHOR IS TOO SHORT!]");
    ct_brush_from(backup);
  }

  ct_cmargin_y(&inner_surface, &inner_surface, 1, 0);
  ct_cmargin_x(&inner_surface, &inner_surface, 1, 1);
  model->author_input.base.render(&model->author_input, &inner_surface);

  ct_cmargin_y(&inner_surface, &inner_surface, 2, 0);
  // ct_cmargin_x(&inner_surface, &inner_surface, 1, 1);
  model->title_input.base.render(&model->title_input, &inner_surface);

  ct_cmargin(&inner_surface, &text_area, 2, 0, 1, 0);
  model->op_input.base.render(&model->op_input, &text_area);

  // KEY ACTIONS
  ct_brush_from(backup);
  ct_brush_fg_hex(HINT_COLOR);
  ct_cwrite_ce(canvas,
               L"[ TAB :: switch focus ] [ F9 :: cancel ] [ F10 :: submit ]");
  ct_brush_from(backup);
}

void thread_creation_form_setup(ThreadCreationFrom *model,
                                const enum Boards for_board) {
  model->base.cleanup = thread_creation_cleanup;
  model->base.handler = thread_creation_handler;
  model->base.render = thread_creation_render;

  model->for_board = for_board;
  model->_error_op = false;
  model->_error_title = false;
  model->_error_author = false;
  model->_disable_input = false;
  model->created_thread = false;

  label_input_setup(&model->title_input, L"[ THREAD TITLE ]", THREAD_MAX_TITLE,
                    40, BLACK_COLOR, BLUE_COLOR, HINT_COLOR);
  label_input_setup(&model->author_input, L"[ AUTHOR ]", AUTHOR_MAX_NAME, 40,
                    BLACK_COLOR, BLUE_COLOR, HINT_COLOR);
  textarea_setup(&model->op_input, L"[ OP ]", BLACK_COLOR, BLUE_COLOR,
                 HINT_COLOR);

  wcsncpy(model->author_input._buffer, DEFAUL_USERNAME, AUTHOR_MAX_NAME);
}
