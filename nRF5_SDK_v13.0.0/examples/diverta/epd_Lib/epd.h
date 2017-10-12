/*
 * epd.h
 *
 *  Created on: 2016/05/13
 *      Author: K.Shoji
 */
#ifndef EPD_H_
#define EPD_H_

#include <stdint.h>

void epd_init(void);
void epd_power_on(void);
void epd_power_off(void);
void epd_clear_display(void);
void epd_update(void);

void epd_test_write_char(void);
void epd_test_graphics(void);

#endif	/* EPD_H_ */
