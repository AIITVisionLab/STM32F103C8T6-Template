#include "app.h"
#include "board.h"
#include "drivers.h"
#include "platform.h"
#include "project_status.h"
#include "services.h"

static void project_boot_or_panic(void);

int main(void)
{
  project_boot_or_panic();

  while (1)
  {
    platform_poll();
    drivers_poll();
    services_poll();
    app_poll();
  }
}

static void project_boot_or_panic(void)
{
  project_status_t status = platform_init();

  if (status != PROJECT_STATUS_OK)
  {
    platform_panic(status);
  }

  status = board_init();
  if (status != PROJECT_STATUS_OK)
  {
    platform_panic(status);
  }

  status = drivers_init();
  if (status != PROJECT_STATUS_OK)
  {
    platform_panic(status);
  }

  status = services_init();
  if (status != PROJECT_STATUS_OK)
  {
    platform_panic(status);
  }

  status = app_init();
  if (status != PROJECT_STATUS_OK)
  {
    platform_panic(status);
  }
}
