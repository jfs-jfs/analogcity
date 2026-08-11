#pragma once

#include "label-input.h"
#include <cursed-tea/core.h>
#include <wchar.h>

typedef struct NicknameForm {
  struct CtModel base;
  LabelInput input;
  bool _error;
  wchar_t *nickname_buffer;
} NicknameForm;

void nickname_form_setup(NicknameForm *model, wchar_t *nickname_buffer);
