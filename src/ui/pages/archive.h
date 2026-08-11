#pragma once

#include "../../config.h"
#include "../../helpers.h"
#include "../components/column.h"
#include "../pages/archive-entry-view.h"
#include <cursed-tea/core.h>
#include <stddef.h>
#include <wchar.h>

typedef struct Archive {
  struct CtModel base;
  Column column;

  wchar_t directory[MAX_PATH_LENGTH + 1];

  ArchiveEntry *entries;
  size_t entries_count;
  bool _initialized_entries;

  size_t _selection_index;

  bool selection;
  ArchiveEntry *selected_entry;

  bool _initialized_entry_view;
  ArchiveEntryView entry_view;

  // not for me to clean
  struct CtModel *title;

} Archive;

void archive_setup(Archive *model, const wchar_t *archive_directory,
                   struct CtModel *title);
