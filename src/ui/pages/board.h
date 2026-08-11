#pragma once

#include "../../config.h"
#include "../../helpers.h"
#include "../components/highway-title.h"
#include "../components/meatspace-title.h"
#include "../components/meta-title.h"
#include "../components/random-title.h"
#include "../components/tech-title.h"
#include "../components/thread-creation-from.h"
#include <cursed-tea/core.h>
#include <stddef.h>
#include <stdint.h>

typedef struct BoardListing {
  struct CtModel base;
  enum Boards board;
  char _board_name[BOARD_MAX_NAME + 1];
  ThreadInfo _threads[THREADS_TO_DISPLAY_PER_BOARD];
  uint8_t _loaded_threads;
  bool _loading, _show_creation_form;
  uint8_t _cursor_at;

  ThreadInfo *selection;

  // titles
  HighwayTitle highway_title;
  MeatSpaceTitle meatspace_title;
  MetaTitle meta_title;
  RandomTitle random_title;
  TechnologyTitle tech_title;

  // Thread creation
  ThreadCreationFrom new_thread_form;
} BoardListing;

void board_listing_setup(BoardListing *model, const enum Boards board);
