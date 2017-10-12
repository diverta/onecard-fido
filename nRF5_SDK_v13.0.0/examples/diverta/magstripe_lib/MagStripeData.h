/*
 * MagStripeData.h
 *
 *  Created on: 2014/11/11
 *      Author: shoji
 */

#ifndef MAGSTRIPEDATA_H_
#define MAGSTRIPEDATA_H_

#include "F2FData.h"
#include "TrackNumber.h"
#include "BitLength.h"

#define MAX_DATA_LENGTH 110

/*
 * Public Function
 */
void MagStripeData_init(void);
void MagStripeData_setTrackNumber(TrackNumber tn);
TrackNumber MagStripeData_getTrackNumber(void);
void MagStripeData_setSyncBitLength(int len);

void MagStripeData_clear(void);
void MagStripeData_add(char c);
void MagStripeData_encode(void);
unsigned char* MagStripeData_getF2FDataAddr(void);
int MagStripeData_getF2FDataBitLength(void);


#endif /* MAGSTRIPEDATA_H_ */
