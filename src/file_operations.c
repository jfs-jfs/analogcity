#include "file_operations.h"
#include "config.h"
#include "helpers.h"
#include <cursed-tea/logger.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

struct FileInfo {
  char name[THREAD_MAX_TITLE + 1];
  char board[BOARD_MAX_NAME + 1];
  time_t modification_time;
};

int compare_by_modification_time(const void *a, const void *b) {
  const struct FileInfo *one = a;
  const struct FileInfo *other = b;

  if (one->modification_time < other->modification_time)
    return 1;
  if (other->modification_time < one->modification_time)
    return -1;

  return strcmp(one->name, other->name);
}

// Expects out to have size AUTHOR_MAX_NAME + 1
void load_thread_author(const char *board, const char *thread_file,
                        wchar_t *out) {

  char author_buffer[AUTHOR_MAX_NAME + 1];
  out[AUTHOR_MAX_NAME] = L'\0';
  author_buffer[AUTHOR_MAX_NAME] = '\0';

  const size_t max_size_path = THREAD_MAX_TITLE + BOARD_MAX_NAME + 2;
  char path[max_size_path];
  snprintf(path, max_size_path, "%s/%s", board, thread_file);

  FILE *fd = fopen(path, "rb");
  if (!fd) {
    log_fmt(LOG_ERR, L"load_thread_author: unable to open thread file '%s'",
            path);
    out[0] = L'\0';
    return;
  }

  int fd_number = fileno(fd);
  if (flock(fd_number, LOCK_SH) == -1) {
    log_fmt(LOG_ERR,
            L"load_thread_author: unable to acquire shared lock for '%s'",
            path);
    fclose(fd);
    out[0] = L'\0';
    return;
  }

  // Skip first line (reply count)
  size_t replies;
  fscanf(fd, "%zu", &replies);
  fgetc(fd); // consume newline

  // Read second line (author name)
  if (!fgets(author_buffer, AUTHOR_MAX_NAME + 1, fd)) {
    log_fmt(LOG_ERR,
            L"load_thread_author: unable to read author from file '%s'", path);
    out[0] = L'\0';
    flock(fd_number, LOCK_UN);
    fclose(fd);
    return;
  }

  if (flock(fd_number, LOCK_UN) == -1) {
    log_fmt(LOG_ERR,
            L"load_thread_author: unable to release shared lock for '%s'",
            path);
  }

  // Strip trailing newline
  size_t len = strlen(author_buffer);
  if (len > 0 && author_buffer[len - 1] == '\n')
    author_buffer[len - 1] = '\0';

  mbstowcs(out, author_buffer, AUTHOR_MAX_NAME + 1);

  fclose(fd);
}

size_t _load_thread_replies_count(const char *board, const char *thread_file) {

  const size_t max_size_path =
      THREAD_MAX_TITLE + BOARD_MAX_NAME + 2; // 1 for separator 1 for \0
  char path[max_size_path];
  snprintf(path, max_size_path, "%s/%s", board, thread_file);

  FILE *fd = fopen(path, "rb");
  if (!fd) {
    log_fmt(LOG_ERR,
            L"_load_thread_replies_count: unable to find thread file '%s'",
            path);
    return 0;
  }

  int fd_number = fileno(fd);
  if (flock(fd_number, LOCK_SH) == -1) {
    log_fmt(
        LOG_ERR,
        L"_load_thread_replies_count: unable to aquire shared lock for '%s' "
        L"thread file",
        path);
    fclose(fd);
    return 0;
  }

  size_t replies = 0;
  if (fscanf(fd, "%zu", &replies) != 1) {
    log_fmt(
        LOG_ERR,
        L"_load_thread_replies_count: unable to read number of replies from "
        L"file '%s'",
        path);
    replies = 0;
  }

  if (flock(fd_number, LOCK_UN) == -1) {
    log_fmt(
        LOG_ERR,
        L"_load_thread_replies_count: unable to release shared lock for '%s' "
        L"thread file",
        path);
    fclose(fd);
    return replies;
  }

  fclose(fd);
  return replies;
}

size_t load_thread_replies_count(const char *board,
                                 const wchar_t *thread_title) {
  size_t title_size = wcstombs(NULL, thread_title, 0);
  if (title_size == (size_t)-1) {
    log_fmt(LOG_ERR, L"load_thread_replies_count: invalid wide character in "
                     L"thread_title");
    return 0;
  }
  char thread_title_char[title_size + 1];
  wcstombs(thread_title_char, thread_title, title_size + 1);

  return _load_thread_replies_count(board, thread_title_char);
}

// out assumed to be an array of ThreadInfo of size THREADS_TO_DISPLAY_PER_BOARD
size_t load_thread_descriptions_all_boards(ThreadInfo *out) {
  char boards[BOARDS_NUMBER][BOARD_MAX_NAME + 1];
  board_names(boards);

  memset(out, 0, sizeof(ThreadInfo) * THREADS_TO_DISPLAY_PER_BOARD);

  struct FileInfo newest[THREADS_TO_DISPLAY_PER_BOARD];
  size_t count = 0;

  for (int b = 0; b < BOARDS_NUMBER; b++) {
    struct stat file_info;
    if (0 != stat(boards[b], &file_info) || !S_ISDIR(file_info.st_mode)) {
      if (0 != mkdir(boards[b], 0755))
        log_fmt(LOG_ERR,
                L"load_thread_descriptions_all_boards: unable to create "
                L"directory for board '%s'",
                boards[b]);
      continue;
    }

    DIR *directory = opendir(boards[b]);
    if (NULL == directory) {
      log_fmt(LOG_ERR,
              L"load_thread_descriptions_all_boards: unable to open "
              L"directory '%s'",
              boards[b]);
      continue;
    }

    struct dirent *current_entry;
    const size_t max_size_path = THREAD_MAX_TITLE + BOARD_MAX_NAME + 2;
    while ((current_entry = readdir(directory)) != NULL) {
      struct stat file_status;
      char path[max_size_path];

      if (strcmp(current_entry->d_name, ".") == 0 ||
          strcmp(current_entry->d_name, "..") == 0)
        continue;

      snprintf(path, max_size_path * sizeof(char), "%s/%s", boards[b],
               current_entry->d_name);

      if (0 != stat(path, &file_status)) {
        log_fmt(LOG_ERR,
                L"load_thread_descriptions_all_boards: unable to stat %s",
                path);
        continue;
      }

      if (S_ISDIR(file_status.st_mode))
        continue;

      if (count < THREADS_TO_DISPLAY_PER_BOARD) {
        strncpy(newest[count].name, current_entry->d_name, THREAD_MAX_TITLE);
        newest[count].name[THREAD_MAX_TITLE] = '\0';
        strncpy(newest[count].board, boards[b], BOARD_MAX_NAME);
        newest[count].board[BOARD_MAX_NAME] = '\0';
        newest[count].modification_time = file_status.st_mtime;
        count++;
        continue;
      }

      size_t oldest_index = 0;
      for (size_t i = 0; i < THREADS_TO_DISPLAY_PER_BOARD; i++)
        if (newest[i].modification_time <
            newest[oldest_index].modification_time)
          oldest_index = i;

      if (file_status.st_mtime > newest[oldest_index].modification_time) {
        strncpy(newest[oldest_index].name, current_entry->d_name,
                THREAD_MAX_TITLE);
        newest[oldest_index].name[THREAD_MAX_TITLE] = '\0';
        strncpy(newest[oldest_index].board, boards[b], BOARD_MAX_NAME);
        newest[oldest_index].board[BOARD_MAX_NAME] = '\0';
        newest[oldest_index].modification_time = file_status.st_mtime;
      }
    }
    closedir(directory);
  }

  qsort(newest, count, sizeof(newest[0]), compare_by_modification_time);

  for (size_t i = 0; i < count; i++) {
    mbstowcs(out[i].thread_title, newest[i].name, THREAD_MAX_TITLE + 1);
    out[i].thread_board = to_board_enum(newest[i].board);
    out[i].last_reply_timestamp = newest[i].modification_time;
    out[i].number_of_replies =
        _load_thread_replies_count(newest[i].board, newest[i].name);
    load_thread_author(newest[i].board, newest[i].name, out[i].thread_author);
  }

  return count;
}

size_t load_threads_descriptions(const char *board, ThreadInfo *out) {
  // Clean array
  memset(out, 0, sizeof(ThreadInfo) * THREADS_TO_DISPLAY_PER_BOARD);

  enum Boards board_enum = to_board_enum(board);

  struct stat file_info;
  // Check if it doesnt exist
  if (0 != stat(board, &file_info) || !S_ISDIR(file_info.st_mode)) {
    if (0 != mkdir(board, 0755))
      log_fmt(LOG_ERR,
              L"load_thread_descriptions: unable to create new directory for "
              L"board '%s'",
              board);
    return 0;
  }

  // Read THREADS_TO_DISPLAY_PER_BOARD # newest files from the directory
  struct FileInfo newest[THREADS_TO_DISPLAY_PER_BOARD];
  size_t count = 0;

  DIR *directory = opendir(board);
  if (NULL == directory) {
    log_fmt(LOG_ERR, L"load_thread_descriptions: unable to open directory '%s'",
            board);
    return 0;
  }

  struct dirent *current_entry;
  const size_t max_size_path =
      THREAD_MAX_TITLE + BOARD_MAX_NAME + 2; // 1 for separator 1 for \0
  while ((current_entry = readdir(directory)) != NULL) {
    struct stat file_status;
    char path[max_size_path];

    if (strcmp(current_entry->d_name, ".") == 0 ||
        strcmp(current_entry->d_name, "..") == 0)
      continue;

    // Build path
    snprintf(path, max_size_path * sizeof(char), "%s/%s", board,
             current_entry->d_name);

    // Get file info
    if (0 != stat(path, &file_status)) {
      log_fmt(LOG_ERR,
              L"load_thread_descriptions: unable to get information of file %s",
              path);
      continue;
    }

    // Skip directories
    if (S_ISDIR(file_status.st_mode))
      continue;

    // Array with empty slots
    if (count < THREADS_TO_DISPLAY_PER_BOARD) {
      strncpy(newest[count].name, current_entry->d_name, THREAD_MAX_TITLE);
      newest[count].name[THREAD_MAX_TITLE] = '\0';
      newest[count].modification_time = file_status.st_mtime;
      count++;
      continue;
    }

    // Find oldest
    size_t oldest_index = 0;
    for (size_t i = 0; i < THREADS_TO_DISPLAY_PER_BOARD; i++)
      if (newest[i].modification_time < newest[oldest_index].modification_time)
        oldest_index = i;

    // Replace oldest
    if (file_status.st_mtime > newest[oldest_index].modification_time) {
      strncpy(newest[oldest_index].name, current_entry->d_name,
              THREAD_MAX_TITLE);
      newest[oldest_index].name[THREAD_MAX_TITLE] = '\0';
      newest[oldest_index].modification_time = file_status.st_mtime;
    }
  }
  closedir(directory);

  // Sort array
  qsort(newest, count, sizeof(newest[0]), compare_by_modification_time);

  // Populate out array
  for (size_t i = 0; i < count; i++) {
    mbstowcs(out[i].thread_title, newest[i].name, THREAD_MAX_TITLE + 1);
    out[i].last_reply_timestamp = newest[i].modification_time;
    out[i].number_of_replies =
        _load_thread_replies_count(board, newest[i].name);
    out[i].thread_board = board_enum;
    load_thread_author(board, newest[i].name, out[i].thread_author);
  }

  return count;
}

void create_thread_file(const char *board, const wchar_t *thread_title,
                        const wchar_t *op, const wchar_t *op_text) {

  size_t title_size = wcstombs(NULL, thread_title, 0);
  if (title_size == (size_t)-1) {
    log_fmt(LOG_ERR,
            L"create_thread_file: invalid wide character in thread_title");
    return;
  }
  char thread_title_char[title_size + 1];
  wcstombs(thread_title_char, thread_title, title_size + 1);

  size_t nick_size = wcstombs(NULL, op, 0);
  if (nick_size == (size_t)-1) {
    log_fmt(LOG_ERR, L"create_thread_file: invalid wide character in nickname");
    return;
  }
  char nickname[nick_size + 1];
  wcstombs(nickname, op, nick_size + 1);

  size_t op_size = wcstombs(NULL, op_text, 0);
  if (op_size == (size_t)-1) {
    log_fmt(LOG_ERR, L"create_thread_file: invalid wide character in op_text");
    return;
  }
  char op_text_char[op_size + 1];
  wcstombs(op_text_char, op_text, op_size + 1);

  const size_t max_size_path = THREAD_MAX_TITLE + BOARD_MAX_NAME + 2;
  char path[max_size_path];
  snprintf(path, max_size_path, "%s/%s", board, thread_title_char);

  FILE *fd = fopen(path, "wb");
  if (!fd) {
    log_fmt(LOG_ERR, L"create_thread_file: unable to create thread file '%s'",
            path);
    return;
  }

  int fd_number = fileno(fd);
  if (flock(fd_number, LOCK_EX) == -1) {
    log_fmt(LOG_ERR,
            L"create_thread_file: unable to acquire exclusive lock for '%s'",
            path);
    fclose(fd);
    return;
  }

  char *folded_op_text = fold_text(op_text, THREAD_MAX_LINE_LENGTH);
  fprintf(fd, "0\n%s\n\n\n%s\n", nickname,
          folded_op_text ? folded_op_text : op_text_char);
  free(folded_op_text);

  if (flock(fd_number, LOCK_UN) == -1) {
    log_fmt(LOG_ERR, L"create_thread_file: unable to release lock for '%s'",
            path);
  }

  fclose(fd);
}

void load_archive_file(const wchar_t *archive_directory, const wchar_t *file,
                       wchar_t **out) {

  *out = NULL;

  size_t archive_dir_size = wcstombs(NULL, archive_directory, 0);
  if (archive_dir_size == (size_t)-1) {
    log_fmt(LOG_ERR,
            L"load_archive_file: invalid wide character in archive_directory");
    return;
  }
  char archive_dir_char[archive_dir_size + 1];
  wcstombs(archive_dir_char, archive_directory, archive_dir_size + 1);

  size_t file_size = wcstombs(NULL, file, 0);
  if (file_size == (size_t)-1) {
    log_fmt(LOG_ERR, L"load_archive_file: invalid wide character in file");
    return;
  }
  char file_char[file_size + 1];
  wcstombs(file_char, file, file_size + 1);

  const size_t max_size_path = MAX_PATH_LENGTH * 2 + 2;
  char path[max_size_path];
  snprintf(path, max_size_path, "%s/%s", archive_dir_char, file_char);

  FILE *fd = fopen(path, "rb");
  if (!fd) {
    log_fmt(LOG_ERR, L"load_archive_file: unable to open archive file '%s'",
            path);
    return;
  }

  int fd_number = fileno(fd);
  if (flock(fd_number, LOCK_SH) == -1) {
    log_fmt(LOG_ERR,
            L"load_archive_file: unable to acquire shared lock for '%s'", path);
    fclose(fd);
    return;
  }

  struct stat file_stat;
  if (fstat(fd_number, &file_stat) == -1) {
    log_fmt(LOG_ERR, L"load_archive_file: unable to stat file '%s'", path);
    flock(fd_number, LOCK_UN);
    fclose(fd);
    return;
  }

  size_t file_size_bytes = file_stat.st_size;
  char *buffer = malloc(file_size_bytes + 1);
  if (!buffer) {
    log_fmt(LOG_ERR, L"load_archive_file: unable to allocate buffer for '%s'",
            path);
    flock(fd_number, LOCK_UN);
    fclose(fd);
    return;
  }

  size_t bytes_read = fread(buffer, 1, file_size_bytes, fd);
  buffer[bytes_read] = '\0';

  flock(fd_number, LOCK_UN);
  fclose(fd);

  char *content_start = buffer;

  size_t wchar_len = mbstowcs(NULL, content_start, 0);
  if (wchar_len == (size_t)-1) {
    log_fmt(LOG_ERR, L"load_archive_file: invalid multibyte sequence in '%s'",
            path);
    free(buffer);
    return;
  }

  *out = malloc((wchar_len + 1) * sizeof(wchar_t));
  if (!*out) {
    log_fmt(LOG_ERR,
            L"load_archive_file: unable to allocate wchar_t buffer for '%s'",
            path);
    free(buffer);
    return;
  }

  mbstowcs(*out, content_start, wchar_len + 1);
  (*out)[wchar_len] = L'\0';

  free(buffer);
}

void load_thread_file(const char *board, const wchar_t *thread_title,
                      wchar_t **out) {

  *out = NULL;

  size_t title_size = wcstombs(NULL, thread_title, 0);
  if (title_size == (size_t)-1) {
    log_fmt(LOG_ERR,
            L"load_thread_file: invalid wide character in thread_title");
    return;
  }
  char thread_title_char[title_size + 1];
  wcstombs(thread_title_char, thread_title, title_size + 1);

  const size_t max_size_path = THREAD_MAX_TITLE + BOARD_MAX_NAME + 2;
  char path[max_size_path];
  snprintf(path, max_size_path, "%s/%s", board, thread_title_char);

  FILE *fd = fopen(path, "rb");
  if (!fd) {
    log_fmt(LOG_ERR, L"load_thread_file: unable to open thread file '%s'",
            path);
    return;
  }

  int fd_number = fileno(fd);
  if (flock(fd_number, LOCK_SH) == -1) {
    log_fmt(LOG_ERR,
            L"load_thread_file: unable to acquire shared lock for '%s'", path);
    fclose(fd);
    return;
  }

  struct stat file_stat;
  if (fstat(fd_number, &file_stat) == -1) {
    log_fmt(LOG_ERR, L"load_thread_file: unable to stat file '%s'", path);
    flock(fd_number, LOCK_UN);
    fclose(fd);
    return;
  }

  size_t file_size = file_stat.st_size;
  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    log_fmt(LOG_ERR, L"load_thread_file: unable to allocate buffer for '%s'",
            path);
    flock(fd_number, LOCK_UN);
    fclose(fd);
    return;
  }

  size_t bytes_read = fread(buffer, 1, file_size, fd);
  buffer[bytes_read] = '\0';

  flock(fd_number, LOCK_UN);
  fclose(fd);

  char *content_start = buffer;
  int newlines_skipped = 0;
  while (*content_start && newlines_skipped < 3) {
    if (*content_start == '\n')
      newlines_skipped++;
    content_start++;
  }

  size_t wchar_len = mbstowcs(NULL, content_start, 0);
  if (wchar_len == (size_t)-1) {
    log_fmt(LOG_ERR, L"load_thread_file: invalid multibyte sequence in '%s'",
            path);
    free(buffer);
    return;
  }

  *out = malloc((wchar_len + 1) * sizeof(wchar_t));
  if (!*out) {
    log_fmt(LOG_ERR,
            L"load_thread_file: unable to allocate wchar_t buffer for '%s'",
            path);
    free(buffer);
    return;
  }

  mbstowcs(*out, content_start, wchar_len + 1);
  (*out)[wchar_len] = L'\0';

  free(buffer);
}

void add_reply_to_thread(const char *board, const wchar_t *thread_title,
                         const wchar_t *author, const wchar_t *reply_body) {

  size_t title_size = wcstombs(NULL, thread_title, 0);
  if (title_size == (size_t)-1) {
    log_fmt(LOG_ERR,
            L"add_reply_to_thread: invalid wide character in thread_title");
    return;
  }
  char thread_title_char[title_size + 1];
  wcstombs(thread_title_char, thread_title, title_size + 1);

  size_t author_size = wcstombs(NULL, author, 0);
  if (author_size == (size_t)-1) {
    log_fmt(LOG_ERR, L"add_reply_to_thread: invalid wide character in author");
    return;
  }
  char author_char[author_size + 1];
  wcstombs(author_char, author, author_size + 1);

  const size_t max_size_path = THREAD_MAX_TITLE + BOARD_MAX_NAME + 2;
  char path[max_size_path];
  snprintf(path, max_size_path, "%s/%s", board, thread_title_char);

  FILE *fd = fopen(path, "r+b");
  if (!fd) {
    log_fmt(LOG_ERR, L"add_reply_to_thread: unable to open thread file '%s'",
            path);
    return;
  }

  int fd_number = fileno(fd);
  if (flock(fd_number, LOCK_EX) == -1) {
    log_fmt(LOG_ERR,
            L"add_reply_to_thread: unable to acquire exclusive lock for '%s'",
            path);
    fclose(fd);
    return;
  }

  size_t replies = 0;
  if (fscanf(fd, "%zu", &replies) != 1) {
    log_fmt(LOG_ERR,
            L"add_reply_to_thread: unable to read reply count from '%s'", path);
    flock(fd_number, LOCK_UN);
    fclose(fd);
    return;
  }
  replies++;

  rewind(fd);
  fprintf(fd, "%zu\n", replies);

  fseek(fd, 0, SEEK_END);

  time_t now = time(NULL);
  struct tm *tm_now = localtime(&now);
  char timestamp[64];
  strftime(timestamp, sizeof(timestamp), "%d/%m/%y %H:%M:%S", tm_now);

  char *folded_reply = fold_text(reply_body, THREAD_MAX_LINE_LENGTH);
  char *colored_reply = add_dialog_colors(folded_reply);

  fprintf(fd, "\n\\Z5[[%s :: %s :: #%zu]]\\Zn\n%s\n", author_char, timestamp,
          replies, colored_reply ? colored_reply : "");

  free(folded_reply);
  free(colored_reply);

  if (flock(fd_number, LOCK_UN) == -1) {
    log_fmt(LOG_ERR, L"add_reply_to_thread: unable to release lock for '%s'",
            path);
  }

  fclose(fd);
}

typedef struct ArchiveScan {
  ArchiveEntry *entries; // NULL when only counting
  size_t max_entries;
  size_t count;
} ArchiveScan;

// Recursively walks fs_dir storing entries with a relative title built from
// rel_prefix (e.g. "Meatspace/ADVICE THREAD"). If scan->entries is NULL it
// only counts. Symlinks are skipped to avoid directory cycles.
static void archive_scan_dir(const char *fs_dir, const wchar_t *rel_prefix,
                             ArchiveScan *scan) {
  DIR *dir = opendir(fs_dir);
  if (NULL == dir) {
    log_fmt(LOG_ERR, L"archive_scan_dir: unable to open directory %s", fs_dir);
    return;
  }

  struct dirent *current_entry;
  while ((current_entry = readdir(dir)) != NULL &&
         scan->count < scan->max_entries) {
    if (strcmp(current_entry->d_name, ".") == 0 ||
        strcmp(current_entry->d_name, "..") == 0)
      continue;

    char path[MAX_PATH_LENGTH + 1];
    snprintf(path, MAX_PATH_LENGTH * sizeof(char), "%s/%s", fs_dir,
             current_entry->d_name);

    struct stat file_status;
    if (0 != lstat(path, &file_status)) {
      log_fmt(LOG_ERR,
              L"archive_scan_dir: unable to get information of file %s", path);
      continue;
    }

    if (S_ISLNK(file_status.st_mode))
      continue;

    if (S_ISDIR(file_status.st_mode)) {
      wchar_t sub_rel[MAX_PATH_LENGTH + 1];
      sub_rel[MAX_PATH_LENGTH] = L'\0';
      if (rel_prefix[0] == L'\0')
        mbstowcs(sub_rel, current_entry->d_name, MAX_PATH_LENGTH);
      else
        swprintf(sub_rel, MAX_PATH_LENGTH + 1, L"%ls/%s", rel_prefix,
                 current_entry->d_name);
      archive_scan_dir(path, sub_rel, scan);
      continue;
    }

    if (!S_ISREG(file_status.st_mode))
      continue;

    if (scan->entries != NULL) {
      wchar_t entry_title[MAX_PATH_LENGTH + 1];
      entry_title[MAX_PATH_LENGTH] = L'\0';

      size_t converted;
      if (rel_prefix[0] == L'\0')
        converted =
            mbstowcs(entry_title, current_entry->d_name, MAX_PATH_LENGTH);
      else
        converted = swprintf(entry_title, MAX_PATH_LENGTH + 1, L"%ls/%s",
                             rel_prefix, current_entry->d_name);

      if (converted == (size_t)-1) {
        log_fmt(
            LOG_ERR,
            L"load_archive_entries: invalid multibyte sequence in path '%s'",
            path);
        scan->entries[scan->count].entry_title[0] = L'\0';
      } else {
        wcsncpy(scan->entries[scan->count].entry_title, entry_title,
                MAX_PATH_LENGTH);
        scan->entries[scan->count].entry_title[MAX_PATH_LENGTH] = L'\0';
      }
    }

    scan->count++;
  }

  closedir(dir);
}

size_t count_archive_entries(const wchar_t *directory) {

  size_t directory_size = wcstombs(NULL, directory, 0);
  if (directory_size == (size_t)-1) {
    log_error(L"count_archive_entries: invalid character in directory name");
    return 0;
  }
  char directory_char[directory_size + 1];
  wcstombs(directory_char, directory, directory_size + 1);

  struct stat file_info;
  if (0 != stat(directory_char, &file_info) || !S_ISDIR(file_info.st_mode)) {
    log_fmt(LOG_ERR,
            L"count_archive_entries: archive directory '%ls' does not exist or "
            L"it is a file",
            directory);
    return 0;
  }

  ArchiveScan scan = {0};
  scan.entries = NULL;
  scan.max_entries = (size_t)-1;
  archive_scan_dir(directory_char, L"", &scan);

  return scan.count;
}

void load_archive_entries(ArchiveEntry *entries, const wchar_t *directory,
                          const size_t max_entries) {
  size_t directory_size = wcstombs(NULL, directory, 0);
  if (directory_size == (size_t)-1) {
    log_error(L"count_archive_entries: invalid character in directory name");
    return;
  }
  char directory_char[directory_size + 1];
  wcstombs(directory_char, directory, directory_size + 1);

  struct stat file_info;
  if (0 != stat(directory_char, &file_info) || !S_ISDIR(file_info.st_mode)) {
    log_fmt(LOG_ERR,
            L"count_archive_entries: archive directory '%ls' does not exist or "
            L"it is a file",
            directory);
    return;
  }

  ArchiveScan scan = {0};
  scan.entries = entries;
  scan.max_entries = max_entries;
  archive_scan_dir(directory_char, L"", &scan);
}
