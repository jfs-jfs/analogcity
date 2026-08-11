#include "nickname-form.h"
#include "../style.h"
#include "events.h"
#include "label-input.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/helpers.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/style/border.h>
#include <cursed-tea/style/brush.h>
#include <ncurses.h>
#include <wchar.h>

void nickname_form_cleanup(void *uncasted_model) {
  NicknameForm *model = uncasted_model;
  ct_subcleanup(&model->input.base);
}

void nickname_form_handler(void *uncasted_model, const struct CtEvent *event) {
  NicknameForm *model = uncasted_model;

  // Exit keys
  if (event->type == KEY_EVENT) {
    switch (event->key) {
    case KEY_F(10): // Save
      if (wcslen(model->input._buffer) < AUTHOR_MIN_NAME) {
        model->_error = true;
        return;
      }
      wcsncpy(model->nickname_buffer, model->input._buffer, AUTHOR_MAX_NAME);
      ct_event_send_custom(CUSTOM_EV_DONE, model);
      break;

    case KEY_F(9): // Cancel
      ct_event_send_custom(CUSTOM_EV_DONE, model);
      break;
    case L'\t':
      if (model->input.focus)
        label_input_unfocus(&model->input);
      else
        label_input_focus(&model->input);
    }
  }

  ct_subhandler(&model->input.base, event);
}

void nickname_form_render(const void *uncasted_model, struct CtCanvas *canvas) {

  const NicknameForm *model = uncasted_model;
  struct CtBrush backup = ct_brush();
  struct CtCanvas panel, inner;

  ct_brush_fg_hex(PINK_COLOR);
  ct_ccutout_c(canvas, &panel, 50, 5);
  ct_cborder(&panel, &inner);
  ct_brush_from(backup);

  if (model->_error) {
    ct_brush_fg_hex(RED_COLOR);
    ct_cwrite(&panel, 2, 0, L"!TOO SHORT!");
    ct_brush_from(backup);
  }

  ct_cwrite_cs(&panel, L"[CHANGE NICK]");
  ct_cwrite_ce(&panel, L"[ TAB :: Focus ] [ F9 :: Cancel] [ F10 :: Save ]");

  ct_cmargin_y(&inner, &inner, 1, 0);
  ct_subrender(&model->input.base, &inner);

  ct_brush_from(backup);
}

void nickname_form_setup(NicknameForm *model, wchar_t *nickname_buffer) {
  model->base.cleanup = nickname_form_cleanup;
  model->base.render = nickname_form_render;
  model->base.handler = nickname_form_handler;

  model->nickname_buffer = nickname_buffer;
  label_input_setup(&model->input, L"[ NICKNAME ]", AUTHOR_MAX_NAME, 20,
                    BLACK_COLOR, BLUE_COLOR, HINT_COLOR);

  wcsncpy(model->input._buffer, nickname_buffer, AUTHOR_MAX_NAME);
}
