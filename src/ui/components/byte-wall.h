#pragma once

#include <cursed-tea/core.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct ByteWall {
  struct CtModel base;
  size_t speed_ms;
  size_t _counter;
  bool _running;
} ByteWall;

void byte_wall_setup(ByteWall *model, const size_t speed_ms);
