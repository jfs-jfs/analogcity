#pragma once

#include <cursed-tea.h>
#include <stddef.h>
typedef struct PhrackTitle {
  struct CtModel base;
} PhrackTitle;

void phrack_title_setup(PhrackTitle *model);
