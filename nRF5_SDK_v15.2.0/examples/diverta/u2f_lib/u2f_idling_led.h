#ifndef U2F_IDLING_LED_H__
#define U2F_IDLING_LED_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void u2f_idling_led_on(uint32_t led_for_idling);
void u2f_idling_led_off(uint32_t led_for_idling);


#ifdef __cplusplus
}
#endif

#endif // U2F_IDLING_LED_H__

/** @} */
