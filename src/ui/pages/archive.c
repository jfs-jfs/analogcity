#include "archive.h"
#include "../../file_operations.h"
#include "../components/events.h"
#include "../style.h"
#include "archive-entry-view.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/helpers.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/border.h>
#include <cursed-tea/style/brush.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

#define ENTRIES_ON_SCREEN 19
#define MAX_ENTRY_SIZE_DISPLAY 56

void *load_archive_entries_parallel(void *uncasted_model) {
  Archive *model = uncasted_model;

  model->entries_count = count_archive_entries(model->directory);
  model->entries = malloc(sizeof(ArchiveEntry) * model->entries_count);
  if (model->entries == NULL) {
    log_fmt(LOG_ERR,
            L"load_archive_entries_parallel: unable to alloc space for entries "
            L"(%zu) from %ls directory",
            model->entries_count, model->directory);
    return NULL;
  }

  load_archive_entries(model->entries, model->directory, model->entries_count);

  if (model->entries_count >= ENTRIES_ON_SCREEN)
    model->_selection_index = 9; // ENTRIES_ON_SCREEN / 2 ;
  else
    model->_selection_index = model->entries_count / 2;

  model->_initialized_entries = true;
  ct_event_send_custom(CUSTOM_EV_UPDATE, model);

  return NULL;
}

void archive_cleanup(void *uncasted_model) {
  Archive *model = uncasted_model;
  ct_subcleanup(&model->column.base);

  if (model->_initialized_entries && NULL != model->entries)
    free(model->entries);

  if (model->_initialized_entry_view)
    ct_subcleanup(&model->entry_view.base);
}

void archive_handler(void *uncasted_model, const struct CtEvent *event) {

  Archive *model = uncasted_model;

  if (model->selection) {
    if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_DONE &&
        event->data == &model->entry_view)
      model->selection = false;
    else
      ct_subhandler(&model->entry_view.base, event);
    return;
  }

  // Normal keybindings
  if (event->type == KEY_EVENT) {
    switch (event->key) {
    case L'q':
    case L'Q':
      ct_event_send_exit();
      break;
    case L'b':
    case L'B':
      ct_event_send_custom(CUSTOM_EV_DONE, model);
      break;

    case L'k':
      model->_selection_index = (model->_selection_index == 0)
                                    ? model->entries_count - 1
                                    : model->_selection_index - 1;
      break;
    case L'j':
      model->_selection_index =
          (model->_selection_index + 1) % model->entries_count;
      break;

    case L'\n':
    case L'\r':
      model->selection = true;
      model->selected_entry = &model->entries[model->_selection_index];
      if (model->_initialized_entry_view)
        ct_subcleanup(&model->entry_view.base);
      archive_entry_view_setup(&model->entry_view, model->directory,
                               model->selected_entry->entry_title);
      model->_initialized_entry_view = true;
      break;
    }
  }
}

void _draw_background(const Archive *model, struct CtCanvas *canvas,
                      struct CtCanvas *out_draw_space) {

  struct CtCanvas title_canvas, body_canvas;

  struct CtBrush backup = ct_brush();

  ct_ccutout(canvas, &title_canvas, 0, 0, canvas->max_x - 1, 7);
  ct_ccutout(canvas, &body_canvas, 0, 7, canvas->max_x - 1, canvas->max_y);
  ct_ccutout(&body_canvas, out_draw_space, 15, 0, body_canvas.max_x - 15,
             body_canvas.max_y);

  if (model->title != NULL) {
    ct_brush_fg_hex(BLUE_COLOR);
    ct_subrender(model->title, &title_canvas);
    ct_brush_from(backup);
  }

  ct_brush_fg_hex(PINK_COLOR);
  ct_cwrite_char_line(canvas, 0, 6, canvas->max_x - 2, 6, L'━');
  ct_brush_fg_hex(BLUE_COLOR);
  ct_cwrite(canvas, canvas->max_x / 2 - 2, 6, L" ✦ ");
  ct_subrender(&model->column.base, &body_canvas);
  ct_cmargin_x(&body_canvas, &body_canvas, 0, body_canvas.max_x - 15);
  ct_subrender(&model->column.base, &body_canvas);
  ct_brush_from(backup);
}

void _draw_entry_selection(const Archive *model, struct CtCanvas *canvas) {

  struct CtBrush backup = ct_brush();
  if (!model->_initialized_entries)
    return;

  size_t start_y = (canvas->max_y - ENTRIES_ON_SCREEN) / 2;
  size_t entries_to_show = model->entries_count > ENTRIES_ON_SCREEN
                               ? ENTRIES_ON_SCREEN
                               : model->entries_count;

  wchar_t buffer[MAX_ENTRY_SIZE_DISPLAY + 1];
  for (size_t i = 0; i < entries_to_show; i++) {

    int entry_index =
        ((int)model->_selection_index) - 9 + i; // ENTRIES_ON_SCREEN / 2;
    if (entry_index < 0)
      entry_index += model->entries_count;
    if (entry_index > model->_selection_index)
      entry_index %= model->entries_count;

    wcsncpy(buffer, model->entries[entry_index].entry_title,
            MAX_ENTRY_SIZE_DISPLAY + 1);
    buffer[MAX_ENTRY_SIZE_DISPLAY] = L'\0';

    // size_t length = wcslen(buffer);
    size_t length = MAX_ENTRY_SIZE_DISPLAY;

    if (entry_index == model->_selection_index)
      ct_brush_fg_hex(PINK_COLOR);

    size_t start_x = (canvas->max_x - length) / 2;
    ct_cwrite(canvas, start_x, start_y + i, buffer);

    if (entry_index == model->_selection_index) {
      ct_cwrite_char(canvas, start_x - 2, start_y + i, L'>');
      ct_cwrite_char(canvas, start_x + length + 1, start_y + i, L'<');
      ct_brush_from(backup);
    }
  }
}

void archive_render(const void *uncasted_model, struct CtCanvas *canvas) {

  const Archive *model = uncasted_model;
  struct CtCanvas text_space;

  _draw_background(model, canvas, &text_space);

  if (!model->selection) {
    ct_cwrite_ce(
        canvas,
        L"[ k/j :: Up/Down ] [ ENTER :: Select ] [ b :: Back ] [ q :: Exit ]");
    _draw_entry_selection(model, &text_space);
  } else {
    ct_subrender(&model->entry_view.base, &text_space);
  }
}

void archive_setup(Archive *model, const wchar_t *archive_directory,
                   struct CtModel *title) {

  model->base.cleanup = archive_cleanup;
  model->base.render = archive_render;
  model->base.handler = archive_handler;

  model->title = title;

  model->selection = false;
  model->_initialized_entries = false;
  model->_initialized_entry_view = false;

  model->_selection_index = 0;
  model->entries_count = 0;
  model->entries = NULL;

  column_setup(&model->column);

  bool directory_valid = true;
  wmemset(model->directory, L'\0', MAX_PATH_LENGTH);
  size_t directory_length = wcslen(archive_directory);
  if (NULL == archive_directory || directory_length == 0) {
    log_error(L"archive_setup: archive_directory is empty or null");
    directory_valid = false;
  } else if (directory_length > MAX_PATH_LENGTH) {
    log_error(L"archive_setup: archive_directory path is too long (max. 255 "
              L"wchar_t)");
    directory_valid = false;
  } else {
    wcsncpy(model->directory, archive_directory, directory_length);
  }

  if (!directory_valid)
    return;

  pthread_t loading_thread;
  pthread_create(&loading_thread, NULL, load_archive_entries_parallel, model);
  pthread_detach(loading_thread);
}
