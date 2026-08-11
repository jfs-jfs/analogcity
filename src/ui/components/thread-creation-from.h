#pragma once

#include "label-input.h"
#include "text-area-input.h"
#include <cursed-tea/core.h>
#include <stdbool.h>

typedef struct ThreadCreationFrom {
  struct CtModel base;
  enum Boards for_board;
  LabelInput title_input, author_input;
  TextAreaInput op_input;
  bool created_thread;

  bool _error_title, _error_op, _error_author, _disable_input;
} ThreadCreationFrom;

void thread_creation_form_setup(ThreadCreationFrom *model,
                                const enum Boards for_board);
