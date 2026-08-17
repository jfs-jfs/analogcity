#pragma once

#include "config.h"
#include <cursed-tea/core.h>
#include <wchar.h>

typedef struct ThreadInfo {
  wchar_t thread_title[THREAD_MAX_TITLE + 1];
  wchar_t thread_author[AUTHOR_MAX_NAME + 1];
  size_t last_reply_timestamp;
  size_t number_of_replies;
  enum Boards thread_board;
} ThreadInfo;

void thread_info_copy(const ThreadInfo *from, ThreadInfo *to);

typedef struct ArchiveEntry {
  wchar_t entry_title[MAX_PATH_LENGTH + 1];
} ArchiveEntry;

void to_board_name(const enum Boards board, char *out);
enum Boards to_board_enum(const char *board);
void board_names(char out[BOARDS_NUMBER][BOARD_MAX_NAME + 1]);

// Folds text so no line exceeds max_width characters.
// Returns a newly allocated char* (caller must free), or NULL on error.
char *fold_text(const wchar_t *text, size_t max_width);
char *add_dialog_colors(const char *text);

void cwrite_parsed_dialog_colors(struct CtCanvas *canvas, const size_t x,
                                 const size_t y, const wchar_t *text);

size_t count_visual_rows(const wchar_t *text, size_t width);
