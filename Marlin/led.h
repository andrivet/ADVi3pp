#ifndef __LEDH

#define __LEDH
#include "Marlin.h"

#if (LED_PIN > -1) 
  void led_status();
  void led_init();

  #define LED_UPDATE_INTERVAL 100
  #define LED_HOTEND_ACTIVE_FLASH 800
  #define LED_ERROR_FLASH 200
  #define LED_STATUS led_status()

#else //no led
  #define LED_STATUS
  FORCE_INLINE void led_status() {};
#endif //LED_PIN > -1

#endif
