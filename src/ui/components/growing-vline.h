#pragma once

#include <cursed-tea/core.h>
#include <stdbool.h>
#include <stddef.h>

enum GrowVLineDirection {
  GVL_UP,
  GVL_DOWN,
};

typedef struct GrowingVLine {
  struct CtModel base;
  size_t tick_speed_ms;
  size_t current_y, _end_y;
  enum GrowVLineDirection dir;
  bool _started;
} GrowingVLine;

void growing_vline_setup(GrowingVLine *model, const size_t tick_speed_ms,
                         const enum GrowVLineDirection dir);
