#include "ui/root.h"
#include "ui/style.h"
#include <cursed-tea/application.h>
#include <cursed-tea/core.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/ui.h>

int main() {
  struct Root root;
  struct CursedSizeGuard guard;
  StylePass stylewrapper;

  ct_app_init();
  ct_logger_level(LOG_ERR);

  root_setup(&root);
  ctu_size_guard_setup(&guard, MIN_UI_X_SIZE, MIN_UI_Y_SIZE,
                       &stylewrapper.base);
  stylepass_setup(&stylewrapper, &root.base);
  ct_app_start(&guard.base);
  return EXIT_SUCCESS;
}
