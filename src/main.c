#include "ui/components/size-enforcer.h"
#include "ui/root.h"
#include "ui/style.h"
#include <cursed-tea/application.h>
#include <cursed-tea/core.h>
#include <cursed-tea/logger.h>

int main() {
  struct Root root;
  SizeEnforcer guard;
  StylePass stylewrapper;

  ct_app_init();
  ct_logger_level(LOG_ERR);

  root_setup(&root);
  size_enforcer_setup(&guard, MIN_UI_X_SIZE, MIN_UI_Y_SIZE, &stylewrapper.base);
  stylepass_setup(&stylewrapper, &root.base);
  ct_app_start(&guard.base);
  return EXIT_SUCCESS;
}
