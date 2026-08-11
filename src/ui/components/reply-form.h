#pragma once

#include "../../helpers.h"
#include "label-input.h"
#include "text-area-input.h"
#include <cursed-tea/core.h>
#include <stdbool.h>
#include <wchar.h>

typedef struct ReplyFrom {
  struct CtModel base;
  enum Boards for_board;
  LabelInput author_input;
  TextAreaInput reply_input;

  wchar_t for_thread[THREAD_MAX_TITLE + 1];
  bool created_reply;
  bool _error_reply, _error_author, _disable_input;
} ReplyFrom;

void reply_form_setup(ReplyFrom *model, const enum Boards for_board,
                      const ThreadInfo *thread);
