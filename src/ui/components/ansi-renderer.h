#pragma once

#include <cursed-tea/core.h>

typedef struct AnsiRenderer {
  struct CtModel base;
  char *_ansi_buffer;
} AnsiRenderer;

void ansi_renderer_setup(AnsiRenderer *model, const char *filename);
