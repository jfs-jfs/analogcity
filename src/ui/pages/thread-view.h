#pragma once

#include "../../config.h"
#include "../../helpers.h"
#include "../components/highway-title.h"
#include "../components/meatspace-title.h"
#include "../components/meta-title.h"
#include "../components/random-title.h"
#include "../components/reply-form.h"
#include "../components/tech-title.h"
#include <cursed-tea/core.h>
#include <stdbool.h>
#include <wchar.h>

typedef struct ThreadView {
  struct CtModel base;
  enum Boards board;
  ThreadInfo thread;

  bool _loaded_contents;
  wchar_t *_dynamic_buffer;
  size_t _scroll_offset;
  size_t _total_lines;
  size_t _buffer_length;
  size_t _view_height;

  // titles
  HighwayTitle highway_title;
  MeatSpaceTitle meatspace_title;
  MetaTitle meta_title;
  RandomTitle random_title;
  TechnologyTitle tech_title;

  // Reply form
  ReplyFrom reply_form;
  bool _show_reply_form, _shown_reply_form;
} ThreadView;

void thread_view_setup(ThreadView *model, const enum Boards board,
                       ThreadInfo *info);
