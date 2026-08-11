#include "text-area-input.h"
#include "../style.h"
#include "events.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/style/brush.h>
#include <stddef.h>
#include <wchar.h>

static void pos_to_display(const wchar_t *buf, size_t buf_len, size_t target_pos,
                           size_t line_width, size_t *out_line,
                           size_t *out_col) {
  size_t line = 0;
  size_t col = 0;
  for (size_t pos = 0; pos < target_pos && pos < buf_len;) {
    if (buf[pos] == L'\n') {
      line++;
      col = 0;
      pos++;
    } else if (col == line_width) {
      line++;
      col = 0;
    } else {
      col++;
      pos++;
    }
  }
  *out_line = line;
  *out_col = col;
}

static size_t display_to_pos(const wchar_t *buf, size_t buf_len,
                             size_t line_width, size_t target_line,
                             size_t target_col) {
  size_t line = 0;
  size_t pos;
  for (pos = 0; pos < buf_len && line < target_line;) {
    size_t i;
    for (i = 0; i < line_width && pos < buf_len && buf[pos] != L'\n'; i++)
      pos++;
    line++;
    if (pos < buf_len && buf[pos] == L'\n')
      pos++;
  }
  if (pos >= buf_len)
    return buf_len;
  for (size_t i = 0; i < target_col && pos < buf_len && buf[pos] != L'\n'; i++)
    pos++;
  return pos;
}

void textarea_cleanup(void *uncasted_model) { (void)uncasted_model; }

void textarea_handler(void *uncasted_model, const struct CtEvent *event) {
  TextAreaInput *model = uncasted_model;

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
  case KEY_BACKSPACE:
  case L'\x7f':
  case L'\b':
    if (model->_caret_pos > 0) {
      wmemmove(&model->_buffer[model->_caret_pos - 1],
               &model->_buffer[model->_caret_pos],
               buffer_length - model->_caret_pos + 1);
      model->_caret_pos--;
    }
    break;

  case KEY_LEFT:
    if (model->_caret_pos > 0)
      model->_caret_pos--;
    break;

  case KEY_RIGHT:
    if (model->_caret_pos < buffer_length)
      model->_caret_pos++;
    break;

  case KEY_UP:
  case KEY_DOWN: {
    if (model->_line_width == 0)
      break;
    size_t cl, cc;
    pos_to_display(model->_buffer, buffer_length, model->_caret_pos,
                   model->_line_width, &cl, &cc);
    size_t target_line =
        event->key == KEY_UP ? (cl > 0 ? cl - 1 : 0) : cl + 1;
    model->_caret_pos = display_to_pos(model->_buffer, buffer_length,
                                       model->_line_width, target_line, cc);
    break;
  }

  case L'\n':
  case L'\r':
    if (buffer_length >= REPLY_MAX_SIZE)
      break;
    wmemmove(&model->_buffer[model->_caret_pos + 1],
             &model->_buffer[model->_caret_pos],
             buffer_length - model->_caret_pos + 1);
    model->_buffer[model->_caret_pos] = L'\n';
    model->_caret_pos++;
    break;

  case L'\f':
  case L'\t':
  case L'\v':
  case L'\0':
    break;

  default:
    if (buffer_length >= REPLY_MAX_SIZE)
      break;
    if (event->key >= KEY_MIN || event->key < 32)
      break;
    wmemmove(&model->_buffer[model->_caret_pos + 1],
             &model->_buffer[model->_caret_pos],
             buffer_length - model->_caret_pos + 1);
    model->_buffer[model->_caret_pos] = event->key;
    model->_caret_pos++;
  }
}

void textarea_render(const void *uncasted_model, struct CtCanvas *canvas) {

  const TextAreaInput *model = uncasted_model;

  struct CtBrush backup = ct_brush();
  struct CtCanvas input;
  wchar_t counter_buffer[10];
  size_t buffer_length = wcslen(model->_buffer);

  if (!model->focus) {
    ct_brush_fg(model->background.r, model->background.g, model->background.b);
    ct_brush_bg(model->foreground.r, model->foreground.g, model->foreground.b);
  } else {
    ct_brush_fg(model->foreground.r, model->foreground.g, model->foreground.b);
    ct_brush_bg(model->background.r, model->background.g, model->background.b);
  }
  // Background label
  ct_cwrite_char_line(canvas, 0, 0, canvas->max_x - 1, 0, L' ');
  ct_cwrite(canvas, 2, 0, model->label_text);

  // Label text
  swprintf(counter_buffer, 10, L"%04zu/%04zu", buffer_length, REPLY_MAX_SIZE);
  counter_buffer[9] = '\0';
  ct_cwrite(canvas, canvas->max_x - 2 - 9, 0, counter_buffer);

  // Background input area
  ct_cmargin(canvas, &input, 1, 1, 0, 1);
  ct_brush_bg(model->input_background.r, model->input_background.g,
              model->input_background.b);
  ct_brush_fg(model->foreground.r, model->foreground.g, model->foreground.b);
  for (size_t i = 0; i < canvas->max_y; i++)
    ct_cwrite_char_line(&input, 0, i, input.max_x - 1, i, L' ');

  // Drawing model->_buffer (Oracle generated and refactored)
  size_t line_width = input.max_x;
  ((TextAreaInput *)uncasted_model)->_line_width = line_width;

  size_t total_lines = 0;
  for (size_t position = 0; position < buffer_length;) {
    size_t line_length = 0;
    while (position < buffer_length && model->_buffer[position] != L'\n' &&
           line_length < line_width) {
      position++;
      line_length++;
    }
    total_lines++;
    if (position < buffer_length && model->_buffer[position] == L'\n')
      position++;
  }

  size_t caret_line = 0;
  size_t caret_column = 0;
  pos_to_display(model->_buffer, buffer_length, model->_caret_pos, line_width,
                 &caret_line, &caret_column);

  ((TextAreaInput *)uncasted_model)->_caret_x = caret_column;
  ((TextAreaInput *)uncasted_model)->_caret_y = caret_line;

  size_t scroll_offset =
      total_lines > input.max_y ? total_lines - input.max_y : 0;
  if (caret_line < scroll_offset)
    scroll_offset = caret_line;
  else if (caret_line >= scroll_offset + input.max_y)
    scroll_offset = caret_line - input.max_y + 1;

  size_t display_line = 0;
  for (size_t pos = 0; pos < buffer_length;) {
    size_t line_start = pos;
    size_t line_len = 0;
    while (pos < buffer_length && model->_buffer[pos] != L'\n' &&
           line_len < line_width) {
      pos++;
      line_len++;
    }

    if (display_line >= scroll_offset &&
        display_line < scroll_offset + input.max_y) {
      wchar_t line_buf[line_width + 1];
      wcsncpy(line_buf, &model->_buffer[line_start], line_len);
      line_buf[line_len] = L'\0';
      ct_cwrite(&input, 0, display_line - scroll_offset, line_buf);
    }

    display_line++;
    if (pos < buffer_length && model->_buffer[pos] == L'\n')
      pos++;
  }

  if (model->focus && model->_blinking && caret_line >= scroll_offset &&
      caret_line < scroll_offset + input.max_y) {
    struct CtBrush saved = ct_brush();
    ct_brush_fg(model->input_background.r, model->input_background.g,
                model->input_background.b);
    ct_brush_bg(model->foreground.r, model->foreground.g,
                model->foreground.b);
    ct_cwrite(&input, caret_column, caret_line - scroll_offset, L" ");
    ct_brush_from(saved);
  }

  ct_brush_from(backup);
}

void textarea_setup(TextAreaInput *model, const wchar_t *label,
                    const char *foreground, const char *background,
                    const char *input_background) {
  model->base.cleanup = textarea_cleanup;
  model->base.render = textarea_render;
  model->base.handler = textarea_handler;

  parse_hex_color(background != NULL ? background : DEFAULT_BG_COLOR,
                  &model->background);
  parse_hex_color(foreground != NULL ? foreground : DEFAULT_FG_COLOR,
                  &model->foreground);
  parse_hex_color(input_background != NULL ? input_background
                                           : DEFAULT_FG_COLOR,
                  &model->input_background);

  model->focus = false;
  model->canceled = false;
  model->_blinking = false;
  model->_caret_pos = 0;
  model->_line_width = 1;

  wmemset(model->label_text, L'\0', LABEL_MAX_SIZE + 1);
  wmemset(model->_buffer, L'\0', REPLY_MAX_SIZE + 1);
  wcsncpy(model->label_text, label, LABEL_MAX_SIZE);
}

void textarea_focus(TextAreaInput *model) {
  model->focus = true;
  ct_event_send_custom(CUSTOM_EV_TICK, model);
}

void textarea_unfocus(TextAreaInput *model) {
  model->focus = false;
  model->_blinking = false;
}
