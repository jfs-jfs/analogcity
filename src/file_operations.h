#pragma once

#include "helpers.h"
#include "ui/pages/board.h"
#include <stddef.h>
#include <wchar.h>

// Assumes *out to be an array of THREADS_TO_DISPLAY_PER_BOARD members
// Returns the number of threads that has loaded
size_t load_threads_descriptions(const char *board, ThreadInfo *out);

// out assumed to be an array of ThreadInfo of size THREADS_TO_DISPLAY_PER_BOARD
// Returns the number of threads loaded (up to THREADS_TO_DISPLAY_PER_BOARD)
size_t load_thread_descriptions_all_boards(ThreadInfo *out);

size_t load_thread_replies_count(const char *board,
                                 const wchar_t *thread_title);

void create_thread_file(const char *board, const wchar_t *thread_title,
                        const wchar_t *op, const wchar_t *op_text);

// Out will be allocated memory and you are responsible of freeing it
void load_thread_file(const char *board, const wchar_t *thread_title,
                      wchar_t **out);

void load_archive_file(const wchar_t *archive_directory, const wchar_t *file,
                       wchar_t **out);

void add_reply_to_thread(const char *board, const wchar_t *thread_title,
                         const wchar_t *author, const wchar_t *reply_body);

size_t count_archive_entries(const wchar_t *directory);
void load_archive_entries(ArchiveEntry *entries, const wchar_t *directory,
                          const size_t max_entries);
