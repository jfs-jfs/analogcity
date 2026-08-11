#pragma once
#include <cursed-tea/core.h>
#include <cursed-tea/style/brush.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

typedef struct Fader {
  struct CtModel base;
  struct CtModel *child;
  struct CtRGB from, to;
  size_t speed_ms, _step;
  bool _running, _done;
} Fader;

void fader_setup(Fader *model, struct CtModel *child, const char *from,
                 const char *to, const size_t speed_ms);
