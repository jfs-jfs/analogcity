#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>

typedef struct SizeEnforcer {
  struct CtModel base;
  size_t min_x, min_y;
  bool _can_render;
  struct CtModel *child;
} SizeEnforcer;

void size_enforcer_setup(SizeEnforcer *model, const size_t min_x,
                         const size_t min_y, struct CtModel *child);
