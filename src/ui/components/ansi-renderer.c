#include "ansi-renderer.h"
#include "../style.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/brush.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

void ansi_renderer_cleanup(void *uncasted_model) {
  AnsiRenderer *model = uncasted_model;
  if (NULL != model->_ansi_buffer)
    free(model->_ansi_buffer);
}

static void handle_sgr(const char *params) {
  // Oracle made

  int nums[16];
  int n = 0;
  const char *p = params;
  while (*p && n < 16) {
    if (*p == ';') {
      p++;
      continue;
    }
    nums[n] = 0;
    while (*p >= '0' && *p <= '9') {
      nums[n] = nums[n] * 10 + (*p - '0');
      p++;
    }
    n++;
  }

  int i = 0;
  while (i < n) {
    if (nums[i] == 0) {
      ct_brush_bg_hex(DEFAULT_BG_COLOR);
      ct_brush_fg_hex(DEFAULT_FG_COLOR);
      i++;
    } else if (nums[i] == 38 && i + 4 < n && nums[i + 1] == 2) {
      ct_brush_fg((uint8_t)nums[i + 2], (uint8_t)nums[i + 3],
                  (uint8_t)nums[i + 4]);
      i += 5;
    } else if (nums[i] == 48 && i + 4 < n && nums[i + 1] == 2) {
      ct_brush_bg((uint8_t)nums[i + 2], (uint8_t)nums[i + 3],
                  (uint8_t)nums[i + 4]);
      i += 5;
    } else if (nums[i] == 39) {
      ct_brush_fg_hex(DEFAULT_FG_COLOR);
      i++;
    } else if (nums[i] == 49) {
      ct_brush_bg_hex(DEFAULT_BG_COLOR);
      i++;
    } else {
      i++;
    }
  }
}

void ansi_renderer_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const AnsiRenderer *model = uncasted_model;
  if (model->_ansi_buffer == NULL)
    return;

  // Oracle made
  const char *buf = model->_ansi_buffer;
  size_t len = strlen(buf);
  size_t i = 0;
  size_t x = 0, y = 0;

  mbstate_t state = {0};

  while (i < len) {
    if (buf[i] == '\x1b' && i + 1 < len && buf[i + 1] == '[') {
      i += 2;
      char params[64];
      size_t pi = 0;
      while (i < len && buf[i] != 'm' && pi < sizeof(params) - 1) {
        params[pi++] = buf[i++];
      }
      params[pi] = '\0';
      if (i < len && buf[i] == 'm')
        i++;
      handle_sgr(params);
    } else if (buf[i] == '\n') {
      x = 0;
      y++;
      i++;
    } else {
      wchar_t wc;
      size_t bytes = mbrtowc(&wc, &buf[i], len - i, &state);
      if (bytes == (size_t)-1 || bytes == (size_t)-2) {
        memset(&state, 0, sizeof(state));
        i++;
      } else if (bytes == 0) {
        i++;
      } else {
        if (y < canvas->max_y && x < canvas->max_x)
          ct_cwrite_char(canvas, x, y, wc);
        x++;
        i += bytes;
      }
    }
  }
}

void ansi_renderer_setup(AnsiRenderer *model, const char *filename) {
  model->base.cleanup = ansi_renderer_cleanup;
  model->base.render = ansi_renderer_render;
  model->base.handler = _empty_handler;

  model->_ansi_buffer = NULL;
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    log_fmt(LOG_ERR, L"ansi_renderer: unable to open file: %s", filename);
    return;
  }

  fseek(fp, 0x00, SEEK_END);
  size_t file_size = ftell(fp);
  fseek(fp, 0x00, SEEK_SET);

  if (file_size == 0) {
    log_fmt(LOG_WARN, L"ansi_renderer: file %s is empty", filename);
    fclose(fp);
    return;
  }

  model->_ansi_buffer = malloc(sizeof(char) * file_size + 1);
  if (!model->_ansi_buffer) {
    log_fmt(LOG_ERR,
            L"ansi_renderer: unable to allocate for constents of file %s",
            filename);
    fclose(fp);
    return;
  }
  model->_ansi_buffer[file_size] = '\0';

  if (file_size != fread(model->_ansi_buffer, 0x01, file_size, fp)) {
    log_fmt(LOG_ERR, L"ansi_renderer: unable to read whole file %s", filename);
    free(model->_ansi_buffer);
    model->_ansi_buffer = NULL;
    fclose(fp);
    return;
  }
  fclose(fp);
}
