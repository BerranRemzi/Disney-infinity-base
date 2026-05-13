#include "platform/stm32f103/usb_base_platform.h"

int main(void)
{
  usb_base_platform_init();

  while (1)
    usb_base_platform_poll();

  return 0;
}
