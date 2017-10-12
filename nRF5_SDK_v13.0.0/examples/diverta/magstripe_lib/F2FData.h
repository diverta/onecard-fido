/*
 * F2FData.h
 *
 *  Created on: 2014/11/10
 *      Author: shoji
 */

#ifndef F2FDATA_H_
#define F2FDATA_H_

#define BUFF_BYTE_SIZE 180

/*
 * Public Function
 */
void F2FData_init(void);
void F2FData_setSyncBitLength(int len);
void F2FData_clear(void);
int F2FData_encode(unsigned char* data, int size, int bitlen);
unsigned char* F2FData_getDataBuffAddr(void);
int F2FData_getBitLength(void);

#endif /* F2FDATA_H_ */
