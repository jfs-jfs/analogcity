#pragma once

#include <cursed-tea.h>

typedef struct AnsiRenderer {
  struct CtModel base;
  char *_ansi_buffer;
} AnsiRenderer;

void ansi_renderer_setup(AnsiRenderer *model, const char *filename);
