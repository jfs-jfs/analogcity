#pragma once

#include "../../config.h"
#include "../../helpers.h"
#include "../components/latest-title.h"
#include <cursed-tea/core.h>
#include <stddef.h>
#include <stdint.h>

typedef struct LatestListing {
  struct CtModel base;
  ThreadInfo _threads[THREADS_TO_DISPLAY_PER_BOARD];
  uint8_t _loaded_threads;
  bool _loading;
  uint8_t _cursor_at;

  ThreadInfo *selection;

  LatestTitle title;
} LatestListing;

void latest_listing_setup(LatestListing *model);
