#pragma once

#include "../../config.h"
#include "../components/column.h"
#include <cursed-tea/core.h>
#include <stddef.h>
#include <wchar.h>

typedef struct ArchiveEntryView {
  struct CtModel base;
  wchar_t archive_directory[MAX_PATH_LENGTH + 1];
  wchar_t entry_file[MAX_PATH_LENGTH + 1];

  bool _loaded_contents;
  wchar_t *_dynamic_buffer;
  size_t _total_lines, _scroll_offset, _buffer_length, _view_height;

} ArchiveEntryView;

void archive_entry_view_setup(ArchiveEntryView *model, const wchar_t *directory,
                              const wchar_t *file);

// AQUI
