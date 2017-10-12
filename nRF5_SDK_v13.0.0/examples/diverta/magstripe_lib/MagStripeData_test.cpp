/*
 * MagStripeData_test.cpp
 *
 *  Created on: 2014/11/11
 *      Author: shoji
 */

#include <gtest/gtest.h>
extern "C"
{
#include "MagStripeData.h"
}


/**
 * Private変数参照
 */
extern int bitLength;
extern TrackNumber TrackNum;
extern unsigned char Data[MAX_DATA_LENGTH + 1];
extern unsigned char EncodedData[MAX_DATA_LENGTH + 2];
extern int DataLength;
extern int EncodedDataLength;
extern unsigned char dataBuff[BUFF_BYTE_SIZE];


// 初期化
TEST(MagStripeDataTest, initialize)
{
	MagStripeData_init();

	ASSERT_EQ(0, DataLength);
	ASSERT_EQ(0, EncodedDataLength);
}


// データ追加(Track1)
TEST(MagStripeDataTest, add1)
{
	MagStripeData_init();

	MagStripeData_setTrackNumber(TRACK_1);

	ASSERT_EQ(0, Data[0]);
	ASSERT_EQ(0, DataLength);

	MagStripeData_add('A');
	ASSERT_EQ('A', Data[0]);
	ASSERT_EQ(1, DataLength);

	MagStripeData_add('B');
	ASSERT_EQ('B', Data[1]);
	ASSERT_EQ(2, DataLength);

	MagStripeData_add('c');
	ASSERT_EQ('C', Data[2]);
	ASSERT_EQ(3, DataLength);

	MagStripeData_add(0x19);
	ASSERT_EQ(0, Data[3]);
	ASSERT_EQ(3, DataLength);

	MagStripeData_add(0x60);
	ASSERT_EQ(0, Data[3]);
	ASSERT_EQ(3, DataLength);

	MagStripeData_clear();
	ASSERT_EQ(0, Data[0]);
	ASSERT_EQ(0, DataLength);
}


// データ追加(Track2)
TEST(MagStripeDataTest, add2)
{
	MagStripeData_init();

	MagStripeData_setTrackNumber(TRACK_2);

	ASSERT_EQ(0, Data[0]);
	ASSERT_EQ(0, DataLength);

	MagStripeData_add('0');
	ASSERT_EQ('0', Data[0]);
	ASSERT_EQ(1, DataLength);

	MagStripeData_add('?');
	ASSERT_EQ('?', Data[1]);
	ASSERT_EQ(2, DataLength);

	MagStripeData_add('/');
	ASSERT_EQ(0, Data[2]);
	ASSERT_EQ(2, DataLength);

	MagStripeData_add('@');
	ASSERT_EQ(0, Data[2]);
	ASSERT_EQ(2, DataLength);
}


// エンコード(Track1)
TEST(MagStripeDataTest, encode1)
{
	MagStripeData_init();

	MagStripeData_setTrackNumber(TRACK_1);

	MagStripeData_add('%');
	MagStripeData_add('A');
	MagStripeData_add('B');
	MagStripeData_add('C');
	MagStripeData_add('1');
	MagStripeData_add('2');
	MagStripeData_add('?');

	ASSERT_EQ(7, DataLength);
	ASSERT_EQ('%', Data[0]);
	ASSERT_EQ('A', Data[1]);
	ASSERT_EQ('B', Data[2]);
	ASSERT_EQ('C', Data[3]);
	ASSERT_EQ('1', Data[4]);
	ASSERT_EQ('2', Data[5]);
	ASSERT_EQ('?', Data[6]);

	MagStripeData_encode();

	ASSERT_EQ(0x45, EncodedData[0]);	// % -> 100 0101
	ASSERT_EQ(0x61, EncodedData[1]);	// A -> 110 0001
	ASSERT_EQ(0x62, EncodedData[2]);	// B -> 110 0010
	ASSERT_EQ(0x23, EncodedData[3]);	// C -> 010 0011
	ASSERT_EQ(0x51, EncodedData[4]);	// 1 -> 101 0001
	ASSERT_EQ(0x52, EncodedData[5]);	// 2 -> 101 0010
	ASSERT_EQ(0x1F, EncodedData[6]);	// ? -> 001 1111
										// 1の数  33 1245
	ASSERT_EQ(0x79, EncodedData[7]);	// LRC  111 1001
}


// エンコード(Track2)
TEST(MagStripeDataTest, encode2)
{
	MagStripeData_init();

	MagStripeData_setTrackNumber(TRACK_2);
	MagStripeData_setSyncBitLength(10);

	MagStripeData_add(';');
	MagStripeData_add('1');
	MagStripeData_add('2');
	MagStripeData_add('3');
	MagStripeData_add('4');
	MagStripeData_add('5');
	MagStripeData_add('?');

	ASSERT_EQ(7, DataLength);
	ASSERT_EQ(';', Data[0]);
	ASSERT_EQ('1', Data[1]);
	ASSERT_EQ('2', Data[2]);
	ASSERT_EQ('3', Data[3]);
	ASSERT_EQ('4', Data[4]);
	ASSERT_EQ('5', Data[5]);
	ASSERT_EQ('?', Data[6]);

	MagStripeData_encode();

	ASSERT_EQ(0x0B, EncodedData[0]);	// ; -> 0 1011
	ASSERT_EQ(0x01, EncodedData[1]);	// 1 -> 0 0001
	ASSERT_EQ(0x02, EncodedData[2]);	// 2 -> 0 0010
	ASSERT_EQ(0x13, EncodedData[3]);	// 3 -> 1 0011
	ASSERT_EQ(0x04, EncodedData[4]);	// 4 -> 0 0100
	ASSERT_EQ(0x15, EncodedData[5]);	// 5 -> 1 0101
	ASSERT_EQ(0x1F, EncodedData[6]);	// ? -> 1 1111
										// 1の数   2345
	ASSERT_EQ(0x15, EncodedData[7]);	// LRC  1 0101

	ASSERT_EQ((10 + 5 * 8 + 10) * 2, F2FData_getBitLength());

	// LSB
	// 0 x 10               ;          1          2          3          4          5          ?          LRC        0 x 10
	// 0 0 0 0 0 0 0 0 0 0  1 1 0 1 0  1 0 0 0 0  0 1 0 0 0  1 1 0 0 1  0 0 1 0 0  1 0 1 0 1  1 1 1 1 1  1 0 1 0 1  0 0 0 0 0 0 0 0 0 0
	// 11001100110011001100 1010110100 1011001100 1101001100 1010110010 1100101100 1011010010 1010101010 1011010010 11001100110011001100
	// [0]     [1]     [2]      [3]      [4]      [5]     [6]      [7]      [8]      [9]      [10]    [11]     [12]     [13]    [14]
	// ↓ 4+4bitに分ける
	// 1100 1100  1100 1100  1100 1010  1101 0010  1100 1100  1101 0011  0010 1011  0010 1100  1011 0010  1101 0010  1010 1010  1010 1101  0010 1100  1100 1100  1100 1100
	// [0]        [1]        [2]        [3]        [4]        [5]        [6]        [7]        [8]        [9]        [10]       [11]       [12]       [13]       [14]

	ASSERT_EQ(0x33, dataBuff[0]);	// 0011 0011
	ASSERT_EQ(0x33, dataBuff[1]);	// 0011 0011
	ASSERT_EQ(0x53, dataBuff[2]);	// 0101 0011
	ASSERT_EQ(0x4B, dataBuff[3]);	// 0100 1011
	ASSERT_EQ(0x33, dataBuff[4]);	// 0011 0011
	ASSERT_EQ(0xCB, dataBuff[5]);	// 1100 1011
	ASSERT_EQ(0xD4, dataBuff[6]);	// 1101 0100
	ASSERT_EQ(0x34, dataBuff[7]);	// 0011 0100
	ASSERT_EQ(0x4D, dataBuff[8]);	// 0100 1101
	ASSERT_EQ(0x4B, dataBuff[9]);	// 0100 1011
	ASSERT_EQ(0x55, dataBuff[10]);	// 0101 0101
	ASSERT_EQ(0xB5, dataBuff[11]);	// 1011 0101
	ASSERT_EQ(0x34, dataBuff[12]);	// 0011 0100
	ASSERT_EQ(0x33, dataBuff[13]);	// 0011 0011
	ASSERT_EQ(0x33, dataBuff[14]);	// 0011 0011
}


// エンコード(JIS II)
TEST(MagStripeDataTest, encodeJIS_2)
{
	MagStripeData_init();

	MagStripeData_setTrackNumber(JIS_2);
	MagStripeData_setSyncBitLength(8);

	MagStripeData_add('A');
	MagStripeData_add('B');
	MagStripeData_add('C');
	MagStripeData_add('1');
	MagStripeData_add('2');

	MagStripeData_encode();

	ASSERT_EQ(8, DataLength);		// addした文字数＋開始+終了+LRCを含んだ長さになる
	ASSERT_EQ(0x7F, Data[0]);
	ASSERT_EQ('A', Data[1]);
	ASSERT_EQ('B', Data[2]);
	ASSERT_EQ('C', Data[3]);
	ASSERT_EQ('1', Data[4]);
	ASSERT_EQ('2', Data[5]);
	ASSERT_EQ(0x7F, Data[6]);

	ASSERT_EQ(0xFF, EncodedData[0]);	// DEL(7F)	-> 1111 1111

	ASSERT_EQ(0x41, EncodedData[1]);	// A(41)	-> 0100 0001
	ASSERT_EQ(0x42, EncodedData[2]);	// B(42)	-> 0100 0010
	ASSERT_EQ(0xC3, EncodedData[3]);	// C(43)	-> 1100 0011
	ASSERT_EQ(0xB1, EncodedData[4]);	// 1(31)	-> 1011 0001
	ASSERT_EQ(0xB2, EncodedData[5]);	// 2(32)	-> 1011 0010
	ASSERT_EQ(0xFF, EncodedData[6]);	// DEL(7F) 	-> 1111 1111
											// 1の数		->  433 1144
	ASSERT_EQ(0x3C, EncodedData[7]);	// LRC		-> 0011 1100

	ASSERT_EQ((8 + 8 * 8 + 8) * 2, F2FData_getBitLength());

	// LSB
	// 0 x 8            DEL              A                B                C                1                2                DEL              LRC              0 x 8
	// 0 0 0 0 0 0 0 0  1 1 1 1 1 1 1 1  1 0 0 0 0 0 1 0  0 1 0 0 0 0 1 0  1 1 0 0 0 0 1 1  1 0 0 0 1 1 0 1  0 1 0 0 1 1 0 1  1 1 1 1 1 1 1 1  0 0 1 1 1 1 0 0  0 0 0 0 0 0 0 0
	// 1100110011001100 1010101010101010 1011001100110100 1101001100110100 1010110011001010 1011001101010010 1101001101010010 1010101010101010 1100101010101100 1100110011001100
	// [0]     [1]      [2]     [3]      [4]     [5]      [6]     [7]      [8]     [9]      [10]    [11]     [12]    [13]     [14]    [15]     [16]    [17]     [18]    [19]
	// ↓ 4+4bitに分ける
	// 1100 1100  1100 1100  1010 1010  1010 1010  1011 0011  0011 0100  1101 0011  0011 0100  1010 1100  1100 1010  1011 0011  0101 0010  1101 0011  0101 0010  1010 1010  1010 1010  1100 1010  1010 1100  1100 1100  1100 1100
	// [0]        [1]        [2]        [3]        [4]        [5]        [6]        [7]        [8]        [9]        [10]       [11]       [12]       [13]       [14]       [15]       [16]       [17]       [18]       [19]

	ASSERT_EQ(0x33, dataBuff[0]);	// 0011 0011
	ASSERT_EQ(0x33, dataBuff[1]);	// 0011 0011
	ASSERT_EQ(0x55, dataBuff[2]);	// 0101 0101
	ASSERT_EQ(0x55, dataBuff[3]);	// 0101 0101
	ASSERT_EQ(0xCD, dataBuff[4]);	// 1100 1101
	ASSERT_EQ(0x2C, dataBuff[5]);	// 0010 1100
	ASSERT_EQ(0xCB, dataBuff[6]);	// 1100 1011
	ASSERT_EQ(0x2C, dataBuff[7]);	// 0010 1100
	ASSERT_EQ(0x35, dataBuff[8]);	// 0011 0101
	ASSERT_EQ(0x53, dataBuff[9]);	// 0101 0011
	ASSERT_EQ(0xCD, dataBuff[10]);	// 1100 1101
	ASSERT_EQ(0x4A, dataBuff[11]);	// 0100 1010
	ASSERT_EQ(0xCB, dataBuff[12]);	// 1100 1011
	ASSERT_EQ(0x4A, dataBuff[13]);	// 0100 1010
	ASSERT_EQ(0x55, dataBuff[14]);	// 0101 0101
	ASSERT_EQ(0x55, dataBuff[15]);	// 0101 0101
	ASSERT_EQ(0x53, dataBuff[16]);	// 0101 0011
	ASSERT_EQ(0x35, dataBuff[17]);	// 0011 0101
	ASSERT_EQ(0x33, dataBuff[18]);	// 0011 0011
	ASSERT_EQ(0x33, dataBuff[19]);	// 0011 0011
}



