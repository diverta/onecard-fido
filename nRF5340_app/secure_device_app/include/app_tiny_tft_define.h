/* 
 * File:   app_tiny_tft_define.h
 * Author: makmorit
 *
 * Created on 2022/06/08, 16:41
 */
#ifndef APP_TINY_TFT_DEFINE_H
#define APP_TINY_TFT_DEFINE_H

#include <drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// TFT制御用GPIO関連
//
#define TFT_RST_NODE        DT_ALIAS(tftrst)
#define TFT_RST_GPIO_LABEL  DT_GPIO_LABEL(TFT_RST_NODE, gpios)
#define TFT_RST_GPIO_PIN    DT_GPIO_PIN(TFT_RST_NODE, gpios)
#define TFT_RST_GPIO_FLAGS  (GPIO_OUTPUT | DT_GPIO_FLAGS(TFT_RST_NODE, gpios))

#define TFT_D_C_NODE        DT_ALIAS(tftdc)
#define TFT_D_C_GPIO_LABEL  DT_GPIO_LABEL(TFT_D_C_NODE, gpios)
#define TFT_D_C_GPIO_PIN    DT_GPIO_PIN(TFT_D_C_NODE, gpios)
#define TFT_D_C_GPIO_FLAGS  (GPIO_OUTPUT | DT_GPIO_FLAGS(TFT_D_C_NODE, gpios))

#define TFT_LED_NODE        DT_ALIAS(tftled)
#define TFT_LED_GPIO_LABEL  DT_GPIO_LABEL(TFT_LED_NODE, gpios)
#define TFT_LED_GPIO_PIN    DT_GPIO_PIN(TFT_LED_NODE, gpios)
#define TFT_LED_GPIO_FLAGS  (GPIO_OUTPUT | DT_GPIO_FLAGS(TFT_LED_NODE, gpios))

#ifdef __cplusplus
}
#endif

#endif /* APP_TINY_TFT_DEFINE_H */
