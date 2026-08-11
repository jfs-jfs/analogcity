#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct ArchiveTitle {
  struct CtModel base;
} ArchiveTitle;

void archive_title_setup(ArchiveTitle *model);
