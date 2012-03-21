#include "led.h"
#if (LED_PIN > -1)
#include "Marlin.h"
#include "temperature.h"

static unsigned long previous_millis_led=0;
static unsigned long previous_millis_toggle=0;

void led_init()
{
    SET_OUTPUT(LED_PIN);
}

void led_status()
{
  if (((millis() - previous_millis_led) < LED_UPDATE_INTERVAL))
    return;
  previous_millis_led=millis();
  if (degTargetHotend(active_extruder) > HEATER_0_MINTEMP)
  {
    if (((millis() - previous_millis_toggle) < LED_HOTEND_ACTIVE_FLASH))
        return;
    previous_millis_toggle=millis();
    TOGGLE(LED_PIN);
  }
  else
  {
    WRITE(LED_PIN, HIGH);
  }
}


#endif //LED_PIN > -1


