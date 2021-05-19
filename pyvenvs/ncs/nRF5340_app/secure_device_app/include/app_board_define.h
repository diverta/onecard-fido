/* 
 * File:   app_board_define.h
 * Author: makmorit
 *
 * Created on 2021/04/05, 11:14
 */
#ifndef APP_BOARD_DEFINE_H
#define APP_BOARD_DEFINE_H

#include <drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// ボタン関連
//
#define SW0_NODE	DT_ALIAS(sw0)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))

#define SW1_NODE	DT_ALIAS(sw1)
#define SW1_GPIO_LABEL	DT_GPIO_LABEL(SW1_NODE, gpios)
#define SW1_GPIO_PIN	DT_GPIO_PIN(SW1_NODE, gpios)
#define SW1_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW1_NODE, gpios))

//
// LED関連
//
#define LED0_NODE	DT_ALIAS(led0)
#define LED0_GPIO_LABEL	DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_GPIO_PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED0_NODE, gpios))

#define LED1_NODE	DT_ALIAS(led1)
#define LED1_GPIO_LABEL	DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1_GPIO_PIN	DT_GPIO_PIN(LED1_NODE, gpios)
#define LED1_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED1_NODE, gpios))

#define LED2_NODE	DT_ALIAS(led2)
#define LED2_GPIO_LABEL	DT_GPIO_LABEL(LED2_NODE, gpios)
#define LED2_GPIO_PIN	DT_GPIO_PIN(LED2_NODE, gpios)
#define LED2_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED2_NODE, gpios))

#define LED3_NODE	DT_ALIAS(led3)
#define LED3_GPIO_LABEL	DT_GPIO_LABEL(LED3_NODE, gpios)
#define LED3_GPIO_PIN	DT_GPIO_PIN(LED3_NODE, gpios)
#define LED3_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED3_NODE, gpios))

#ifdef __cplusplus
}
#endif

#endif /* APP_BOARD_DEFINE_H */
