#pragma once

#include "../../config.h"
#include <cursed-tea/core.h>
#include <cursed-tea/style/brush.h>
#include <stddef.h>
#include <wchar.h>

typedef struct TextAreaInput {
  struct CtModel base;
  bool focus, canceled;
  wchar_t label_text[LABEL_MAX_SIZE + 1];
  struct CtRGB foreground, background, input_background;

  bool _blinking;
  wchar_t _buffer[REPLY_MAX_SIZE + 1];
  size_t _caret_pos;
  size_t _caret_x, _caret_y;
  size_t _line_width;
} TextAreaInput;

void textarea_focus(TextAreaInput *model);
void textarea_unfocus(TextAreaInput *model);

void textarea_setup(TextAreaInput *model, const wchar_t *label,
                    const char *foreground, const char *background,
                    const char *input_background);
