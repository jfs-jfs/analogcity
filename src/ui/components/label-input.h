#pragma once

#include "../../config.h"
#include <cursed-tea/core.h>
#include <cursed-tea/style/brush.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

typedef struct LabelInput {
  struct CtModel base;
  wchar_t _label_text[LABEL_MAX_SIZE + 1];
  wchar_t _buffer[LINE_INPUT_MAX_SIZE + 1];
  bool _blinking;
  size_t _offset;

  struct CtRGB foreground, background, input_background;
  size_t max_input, max_visible_input;
  bool focus;
} LabelInput;

void label_input_focus(LabelInput *model);
void label_input_unfocus(LabelInput *model);

void label_input_setup(LabelInput *model, const wchar_t *label,
                       const size_t max_input, const size_t max_visible_input,
                       const char *foreground, const char *background,
                       const char *input_background);
