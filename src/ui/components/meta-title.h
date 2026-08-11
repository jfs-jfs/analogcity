#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct MetaTitle {
  struct CtModel base;
} MetaTitle;

void meta_title_setup(MetaTitle *model);
