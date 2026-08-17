#include "reply-form.h"
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

void *add_reply_to_thread_in_background(void *uncasted_model) {
  ReplyFrom *model = uncasted_model;
  char board_name[BOARD_MAX_NAME + 1];
  to_board_name(model->for_board, board_name);
  board_name[BOARD_MAX_NAME] = '\0';
  add_reply_to_thread(board_name, model->for_thread,
                      model->author_input._buffer, model->reply_input._buffer);
  model->created_reply = true;
  ct_event_send_custom_delayed(CUSTOM_EV_DONE, 10, uncasted_model);
  return NULL;
}

void reply_form_cleanup(void *uncasted_model) {

  ReplyFrom *model = uncasted_model;

  ct_subcleanup(&model->author_input.base);
  ct_subcleanup(&model->reply_input.base);
}

void reply_form_handler(void *uncasted_model, const struct CtEvent *event) {
  ReplyFrom *model = uncasted_model;

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
        textarea_focus(&model->reply_input);
      } else {
        textarea_unfocus(&model->reply_input);
        label_input_focus(&model->author_input);
      }
      break;
    case KEY_F(10):
      // Fire without creating the file
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case KEY_F(2):
      if (wcslen(model->reply_input._buffer) <= REPLY_MIN_SIZE) {
        model->_error_author = false;
        model->_error_reply = true;
        return;
      }
      if (wcslen(model->author_input._buffer) <= AUTHOR_MIN_NAME) {
        model->_error_reply = false;
        model->_error_author = true;
        return;
      }
      model->_disable_input = true;
      pthread_t add_reply_thread;
      pthread_create(&add_reply_thread, NULL, add_reply_to_thread_in_background,
                     model);
      pthread_detach(add_reply_thread);
      break;
    }
  }

  model->author_input.base.handler(&model->author_input, event);
  model->reply_input.base.handler(&model->reply_input, event);
}
void reply_form_render(const void *uncasted_model, struct CtCanvas *canvas) {

  const ReplyFrom *model = uncasted_model;

  struct CtBrush backup = ct_brush();
  struct CtCanvas surface, inner_surface, text_area;
  ct_ccutout_c(canvas, &surface, canvas->max_x - 2, 24);
  ct_cfill(&surface, L' ');

  ct_brush_fg_hex(PINK_COLOR);
  ct_cborder(&surface, &inner_surface);

  ct_brush_from(backup);
  ct_brush_attr(A_BOLD);
  ct_cwrite(&surface, 3, 0, L"[NEW THREAD FORM]");

  if (model->_error_reply) {
    ct_brush_fg_hex(RED_COLOR);
    ct_cwrite(&surface, 21, 0, L"[!REPLY TEXT IS TOO SHORT!]");
    ct_brush_from(backup);
  } else if (model->_error_author) {
    ct_brush_fg_hex(RED_COLOR);
    ct_cwrite(&surface, 21, 0, L"[!AUTHOR IS TOO SHORT!]");
    ct_brush_from(backup);
  }

  ct_cmargin_y(&inner_surface, &inner_surface, 1, 0);
  ct_cmargin_x(&inner_surface, &inner_surface, 1, 1);
  model->author_input.base.render(&model->author_input, &inner_surface);

  ct_cmargin(&inner_surface, &text_area, 2, 0, 1, 0);
  model->reply_input.base.render(&model->reply_input, &text_area);

  // KEY ACTIONS
  ct_brush_from(backup);
  ct_brush_fg_hex(HINT_COLOR);
  ct_cwrite_ce(canvas, L"[ TAB :: Focus ] [ F2 :: Submit ] [ F10 :: Cancel ]");
  ct_brush_from(backup);
}

void reply_form_setup(ReplyFrom *model, const enum Boards for_board,
                      const ThreadInfo *thread) {
  model->base.cleanup = reply_form_cleanup;
  model->base.handler = reply_form_handler;
  model->base.render = reply_form_render;

  model->_error_reply = false;
  model->_error_author = false;
  model->_disable_input = false;
  model->created_reply = false;
  model->for_board = for_board;
  wcsncpy(model->for_thread, thread->thread_title, THREAD_MAX_TITLE);

  label_input_setup(&model->author_input, L"[ AUTHOR ]", AUTHOR_MAX_NAME, 40,
                    BLACK_COLOR, BLUE_COLOR, HINT_COLOR);
  textarea_setup(&model->reply_input, L"[ REPLY ]", BLACK_COLOR, BLUE_COLOR,
                 HINT_COLOR);

  wcsncpy(model->author_input._buffer, DEFAUL_USERNAME, AUTHOR_MAX_NAME);
}
