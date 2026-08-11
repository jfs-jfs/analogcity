#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>

typedef struct Column {
  struct CtModel base;
  size_t counter;
} Column;

void column_setup(Column *model);
