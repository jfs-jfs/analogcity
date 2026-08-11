#include "label-input.h"
#include "../style.h"
#include "events.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/brush.h>
#include <ncurses.h>
#include <stddef.h>
#include <wchar.h>

#define MAX_VISIBLE_INPUT_WIDTH 40

void label_input_cleanup(void *uncasted_model) { (void)uncasted_model; }

void label_input_handler(void *uncasted_model, const struct CtEvent *event) {
  LabelInput *model = uncasted_model;

  if (!model->focus)
    return;

  // Blinking
  if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_TICK &&
      event->data == uncasted_model) {
    model->_blinking = !model->_blinking;
    ct_event_send_custom_delayed(CUSTOM_EV_TICK, 400, uncasted_model);
    return;
  }

  // Keys
  if (event->type != KEY_EVENT)
    return;

  size_t buffer_length = wcslen(model->_buffer);

  switch (event->key) {
  case L'\n':
  case L'\r':
    ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
    break;
  case KEY_BACKSPACE:
  case L'\x7f':
  case L'\b':
    if (buffer_length > 0) {
      model->_buffer[buffer_length - 1] = L'\0';
      if (model->_offset > 0)
        model->_offset--;
    }
    break;

  case L'\f':
  case L'\t':
  case L'\v':
  case L'\0':
    break;

  default:
    if (buffer_length >= model->max_input)
      break;
    if (event->key >= KEY_MIN || event->key < 32)
      break;
    model->_buffer[buffer_length] = event->key;
    if (buffer_length + 1 > model->_offset + model->max_visible_input)
      model->_offset++;
  }
}

void label_input_render(const void *uncasted_model, struct CtCanvas *canvas) {

  const LabelInput *model = uncasted_model;
  struct CtBrush backup = ct_brush();

  size_t buffer_length = wcslen(model->_buffer);
  size_t start_input_x = canvas->max_x - 1 - 2 - 2 - model->max_visible_input;

  // Visible buffer
  wchar_t visible_buffer[LINE_INPUT_MAX_SIZE + 1];
  size_t render_offset = model->focus ? model->_offset : 0;
  if (buffer_length > render_offset) {
    wcsncpy(visible_buffer, model->_buffer + render_offset,
            model->max_visible_input);
    visible_buffer[model->max_visible_input] = L'\0';
  } else {
    visible_buffer[0] = L'\0';
  }

  if (!model->focus) {
    ct_brush_fg(model->background.r, model->background.g, model->background.b);
    ct_brush_bg(model->foreground.r, model->foreground.g, model->foreground.b);
    ct_cwrite_char_line(canvas, 0, 0, canvas->max_x - 1, 0, L' ');
    ct_cwrite(canvas, 2, 0, model->_label_text);

    ct_brush_bg(model->input_background.r, model->input_background.g,
                model->input_background.b);
    ct_brush_fg(model->foreground.r, model->foreground.g, model->foreground.b);
    ct_cwrite_char_line(canvas, start_input_x, 0, canvas->max_x - 3, 0, L' ');
    ct_cwrite(canvas, start_input_x + 1, 0, visible_buffer);
  } else {
    ct_brush_fg(model->foreground.r, model->foreground.g, model->foreground.b);
    ct_brush_bg(model->background.r, model->background.g, model->background.b);
    ct_cwrite_char_line(canvas, 0, 0, canvas->max_x - 1, 0, L' ');
    ct_cwrite(canvas, 2, 0, model->_label_text);

    // ct_brush_fg(model->background.r, model->background.g,
    // model->background.b);
    ct_brush_bg(model->input_background.r, model->input_background.g,
                model->input_background.b);
    ct_brush_fg(model->foreground.r, model->foreground.g, model->foreground.b);
    ct_cwrite_char_line(canvas, start_input_x, 0, canvas->max_x - 3, 0, L' ');
    ct_cwrite(canvas, start_input_x + 1, 0, visible_buffer);

    if (model->_blinking) {
      size_t cursor_visible = buffer_length - model->_offset;
      if (cursor_visible <= model->max_visible_input)
        ct_cwrite_char(canvas, start_input_x + 1 + cursor_visible, 0, L'|');
    }
  }
  ct_brush_from(backup);
}

void label_input_setup(LabelInput *model, const wchar_t *label,
                       const size_t max_input, const size_t max_visible_input,
                       const char *foreground, const char *background,
                       const char *input_background) {
  model->base.cleanup = label_input_cleanup;
  model->base.render = label_input_render;
  model->base.handler = label_input_handler;

  model->focus = false;
  parse_hex_color(background != NULL ? background : DEFAULT_BG_COLOR,
                  &model->background);
  parse_hex_color(foreground != NULL ? foreground : DEFAULT_FG_COLOR,
                  &model->foreground);
  parse_hex_color(input_background != NULL ? input_background
                                           : DEFAULT_FG_COLOR,
                  &model->input_background);
  model->max_visible_input = max_visible_input < MAX_VISIBLE_INPUT_WIDTH
                                 ? max_visible_input
                                 : MAX_VISIBLE_INPUT_WIDTH;
  model->max_input =
      max_input < LINE_INPUT_MAX_SIZE ? max_input : LINE_INPUT_MAX_SIZE;
  wmemset(model->_label_text, L'\0', LABEL_MAX_SIZE + 1);
  wmemset(model->_buffer, L'\0', LINE_INPUT_MAX_SIZE + 1);
  wcsncpy(model->_label_text, label, LABEL_MAX_SIZE);
  model->_blinking = false;
  model->_offset = 0;
}

void label_input_focus(LabelInput *model) {
  model->focus = true;
  ct_event_send_custom(CUSTOM_EV_TICK, model);
}

void label_input_unfocus(LabelInput *model) {
  model->focus = false;
  model->_blinking = false;
}
