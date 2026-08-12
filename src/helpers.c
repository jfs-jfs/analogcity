#include "helpers.h"
#include "config.h"
#include <ctype.h>
#include <cursed-tea/logger.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Assumes out to be of BOARD_MAX_NAME of length
void to_board_name(const enum Boards board, char *out) {
  switch (board) {
  case BOARD_HIGHWAY:
    strncpy(out, "boards/highway", BOARD_MAX_NAME);
    return;
  case BOARD_MEATSPACE:
    strncpy(out, "boards/meatspace", BOARD_MAX_NAME);
    return;
  case BOARD_RANDOM:
    strncpy(out, "boards/random", BOARD_MAX_NAME);
    return;
  case BOARD_TECHNOLOGY:
    strncpy(out, "boards/technology", BOARD_MAX_NAME);
    return;
  case BOARD_META:
    strncpy(out, "boards/meta", BOARD_MAX_NAME);
    return;
  }
}

char *fold_text(const wchar_t *text, size_t max_width) {

  // Oracle generated
  size_t len = wcslen(text);
  wchar_t *folded = malloc((len * 2 + 1) * sizeof(wchar_t));
  if (!folded)
    return NULL;

  size_t j = 1;
  size_t line_start = 0;
  size_t last_space = 0;
  size_t col = 1;
  folded[0] = L' ';
  size_t consecutive_newlines = 0;

  for (size_t i = 0; i < len; i++) {
    wchar_t c = text[i];

    if (c == L'\n') {
      if (consecutive_newlines < 2) {
        folded[j++] = c;
        folded[j++] = L' ';
        line_start = j - 1;
        col = 1;
        last_space = (size_t)-1;
      }
      consecutive_newlines++;
      continue;
    }
    consecutive_newlines = 0;

    if (c == L' ')
      last_space = j;

    folded[j++] = c;
    col++;

    if (col > max_width) {
      if (last_space != (size_t)-1 && last_space > line_start) {
        memmove(&folded[last_space + 2], &folded[last_space + 1],
                (j - last_space - 1) * sizeof(wchar_t));
        folded[last_space] = L'\n';
        folded[last_space + 1] = L' ';
        j++;
        col = j - (last_space + 1);
        line_start = last_space + 1;
        last_space = (size_t)-1;
        for (size_t k = line_start; k < j; k++)
          if (folded[k] == L' ')
            last_space = k;
      } else {
        size_t break_pos = line_start + max_width;
        size_t to_move = j - break_pos;
        memmove(&folded[break_pos + 2], &folded[break_pos],
                to_move * sizeof(wchar_t));
        folded[break_pos] = L'\n';
        folded[break_pos + 1] = L' ';
        j += 2;
        line_start = break_pos + 1;
        col = j - line_start;
        last_space = (size_t)-1;
        for (size_t k = line_start; k < j; k++)
          if (folded[k] == L' ')
            last_space = k;
      }
    }
  }

  folded[j] = L'\0';

  size_t mb_len = wcstombs(NULL, folded, 0);
  if (mb_len == (size_t)-1) {
    log_fmt(LOG_ERR, L"fold_text: invalid wide character sequence");
    free(folded);
    return NULL;
  }

  char *result = malloc(mb_len + 1);
  if (!result) {
    free(folded);
    return NULL;
  }

  wcstombs(result, folded, mb_len + 1);
  free(folded);
  return result;
}

void thread_info_copy(const ThreadInfo *from, ThreadInfo *to) {
  wcsncpy(to->thread_title, from->thread_title, THREAD_MAX_TITLE);
  wcsncpy(to->thread_author, from->thread_author, AUTHOR_MAX_NAME);
  to->last_reply_timestamp = from->last_reply_timestamp;
  to->number_of_replies = from->number_of_replies;
}

// assumed out to be char out[BOARDS_NUMBER][BOARD_MAX_NAME + 1]
void board_names(char out[BOARDS_NUMBER][BOARD_MAX_NAME + 1]) {
  strncpy(out[BOARD_HIGHWAY], "boards/highway", BOARD_MAX_NAME);
  strncpy(out[BOARD_MEATSPACE], "boards/meatspace", BOARD_MAX_NAME);
  strncpy(out[BOARD_RANDOM], "boards/random", BOARD_MAX_NAME);
  strncpy(out[BOARD_TECHNOLOGY], "boards/technology", BOARD_MAX_NAME);
  strncpy(out[BOARD_META], "boards/meta", BOARD_MAX_NAME);
}

enum Boards to_board_enum(const char *board) {
  if (0 == strcmp(board, "boards/highway"))
    return BOARD_HIGHWAY;
  else if (0 == strcmp(board, "boards/meatspace"))
    return BOARD_MEATSPACE;
  else if (0 == strcmp(board, "boards/technology"))
    return BOARD_TECHNOLOGY;
  else if (0 == strcmp(board, "boards/random"))
    return BOARD_RANDOM;
  else if (0 == strcmp(board, "boards/meta"))
    return BOARD_META;

  return BOARD_META;
}

void cwrite_parsed_dialog_colors(struct CtCanvas *canvas, const size_t x,
                                 const size_t y, const wchar_t *text) {

  struct CtBrush backup = ct_brush();

  // Oracle produced
  size_t cx = x, cy = y;

  for (const wchar_t *p = text; *p; p++) {
    if (*p == L'\\' && *(p + 1) == L'Z') {
      p++;
      p++;
      if (!*p)
        break;

      switch (*p) {
      case L'0':
        ct_brush_fg_hex("#000000");
        break;
      case L'1':
        ct_brush_fg_hex("#AA0000");
        break;
      case L'2':
        ct_brush_fg_hex("#00AA00");
        break;
      case L'3':
        ct_brush_fg_hex("#AA5500");
        break;
      case L'4':
        ct_brush_fg_hex("#0000AA");
        break;
      case L'5':
        ct_brush_fg_hex("#AA00AA");
        break;
      case L'6':
        ct_brush_fg_hex("#00AAAA");
        break;
      case L'7':
        ct_brush_fg_hex("#AAAAAA");
        break;
      case L'b':
        ct_brush_add_attr(A_BOLD);
        break;
      case L'B': {
        struct CtBrush cur = ct_brush();
        cur.attribute &= ~A_BOLD;
        ct_brush_attr(cur.attribute);
        break;
      }
      case L'r':
        ct_brush_add_attr(A_REVERSE);
        break;
      case L'R': {
        struct CtBrush cur = ct_brush();
        cur.attribute &= ~A_REVERSE;
        ct_brush_attr(cur.attribute);
        break;
      }
      case L'u':
        ct_brush_add_attr(A_UNDERLINE);
        break;
      case L'U': {
        struct CtBrush cur = ct_brush();
        cur.attribute &= ~A_UNDERLINE;
        ct_brush_attr(cur.attribute);
        break;
      }
      case L'n':
        ct_brush_from(backup);
        break;
      }
    } else if (*p == L'\n') {
      cx = x;
      cy++;
    } else {
      ct_cwrite_char(canvas, cx, cy, *p);
      cx++;
    }
  }

  ct_brush_from(backup); // always at the end of the line
}

char *add_dialog_colors(const char *text) {

  // Oracle made
  size_t len = strlen(text);
  char *colored = malloc(len * 8 + 1);
  if (!colored)
    return NULL;

  size_t j = 0;
  for (size_t i = 0; i < len; i++) {
    if (text[i] == '#' && isdigit((unsigned char)text[i + 1])) {
      size_t digits = 0;
      while (isdigit((unsigned char)text[i + 1 + digits]))
        digits++;
      strcpy(&colored[j], "\\Z5\\Zb");
      j += 6;
      for (size_t k = 0; k <= digits; k++)
        colored[j++] = text[i + k];
      colored[j++] = '\\';
      colored[j++] = 'Z';
      colored[j++] = 'n';
      i += digits;
    } else if (text[i] == '>') {
      size_t n = i + 1;
      while (n < len && text[n] != '\n')
        n++;
      strcpy(&colored[j], "\\Z2\\Zb");
      j += 6;
      for (size_t k = i; k < n; k++)
        colored[j++] = text[k];
      if (n < len)
        colored[j++] = '\n';
      colored[j++] = '\\';
      colored[j++] = 'Z';
      colored[j++] = 'n';
      i = n;
    } else {
      colored[j++] = text[i];
    }
  }

  colored[j] = '\0';
  return colored;
}
