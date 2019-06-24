/* 
 * File:   fido_board.h
 * Author: makmorit
 *
 * Created on 2019/06/18, 10:12
 */

#ifndef FIDO_BOARD_H
#define FIDO_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

// 関数群
void fido_processing_led_timedout_handler(void);
void fido_idling_led_timedout_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_H */

