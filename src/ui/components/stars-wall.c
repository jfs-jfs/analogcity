#include "stars-wall.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/ui/common.h>

void stars_wall_render(const void *uncasted_model, struct CtCanvas *canvas) {
  log_trace();

  const StarsWall *model = uncasted_model;

  for (size_t i = 0; i * model->spacing_y < canvas->max_y; i++)
    for (size_t j = 0; j * model->spacing_x < canvas->max_x; j++)
      ct_cwrite_char(canvas, j * model->spacing_x, i * model->spacing_y, L'+');
}

void stars_wall_setup(StarsWall *model, const size_t spacing_x,
                      const size_t spacing_y) {
  model->base.cleanup = _empty_cleanup;
  model->base.handler = _empty_handler;
  model->base.render = stars_wall_render;

  model->spacing_x = spacing_x;
  model->spacing_y = spacing_y;
}
