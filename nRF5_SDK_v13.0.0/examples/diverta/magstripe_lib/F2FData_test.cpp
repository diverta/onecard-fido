/*
 * F2FData_test.cpp
 *
 *  Created on: 2014/11/10
 *      Author: shoji
 */

#include <gtest/gtest.h>
extern "C"
{
#include "F2FData.h"
}


/**
 * Private変数参照
 */
extern int syncBitLength;
extern int bitLength;
extern unsigned char dataBuff[];

/**
 * 初期化
 */
TEST(F2FDataTest, initialize)
{
	F2FData_init();
	ASSERT_EQ(20, syncBitLength);
	ASSERT_EQ(0, bitLength);
}


/**
 * エンコード1
 */
TEST(F2FDataTest, encode1)
{
	unsigned char data[] = {
			0x12,		// 001 0010
			0x34		// 011 0100
	};

	F2FData_init();
	F2FData_setSyncBitLength(10);
	ASSERT_EQ(10, syncBitLength);

	int bitLen = F2FData_encode(data, 2, 7);
	ASSERT_EQ((10 + 7 * 2 + 10) * 2, bitLen);

	// ←LSB
	// 0 x 10               0x12           0x34           0 x 10
	// 0 0 0 0 0 0 0 0 0 0  0 1 0 0 1 0 0  0 0 1 0 1 1 0  0 0 0 0 0 0 0 0 0 0
	// ↓F2F符号化
	// 11001100110011001100 11010011010011 00110100101011 00110011001100110011
	// ↓4+4bitに分ける
	// 1100 1100  1100 1100  1100 1101  0011 0100  1100 1101  0010 1011  0011 0011  0011 0011  0011
	// [0]        [1]        [2]        [3]        [4]        [5]        [6]        [7]        [8]

	ASSERT_EQ(0x33, dataBuff[0]);	// 0011 0011
	ASSERT_EQ(0x33, dataBuff[1]);	// 0011 0011
	ASSERT_EQ(0xB3, dataBuff[2]);	// 1011 0011
	ASSERT_EQ(0x2C, dataBuff[3]);	// 0010 1100
	ASSERT_EQ(0xB3, dataBuff[4]);	// 1011 0011
	ASSERT_EQ(0xD4, dataBuff[5]);	// 1101 0100
	ASSERT_EQ(0xCC, dataBuff[6]);	// 1100 1100
	ASSERT_EQ(0xCC, dataBuff[7]);	// 1100 1100
	ASSERT_EQ(0x0C, dataBuff[8]);	// 0000 1100
}
