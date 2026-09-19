#pragma once

#include <cursed-tea.h>
#include <stddef.h>
typedef struct Title {
  struct CtModel base;
} Title;

void title_setup(Title *model);
