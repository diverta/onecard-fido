/*
 * F2FData.c
 *
 *  Created on: 2014/11/10
 *      Author: shoji
 */

#include "F2FData.h"


/*
 * Private Variable
 */

// テストコード側からPrivate変数を外部参照する為に、単体テスト時のみstaticを外します
#ifdef GTEST
#define STATIC
#else
#define STATIC static
#endif

static const int buffByteSize = BUFF_BYTE_SIZE;
static const int defaultSyncBitLength = 20;

STATIC int syncBitLength;
STATIC int bitLength;
STATIC unsigned char dataBuff[BUFF_BYTE_SIZE];

/*
 * Private Function
 */
int addSyncBit(int currLevel);


/**
 * 初期化 (コンストラクタの代わり
 */
void F2FData_init(void)
{
	int i;
	for (i = 0; i < buffByteSize; i++)
	{
		dataBuff[i] = 0;
	}

	bitLength = 0;
	syncBitLength = defaultSyncBitLength;
}


/**
 * データクリア
 */
void F2FData_clear(void)
{
	int i;
	for (i = 0; i < buffByteSize; i++)
	{
		dataBuff[i] = 0;
	}

	bitLength = 0;
}


/**
 * 同期ビット数をセットする
 * @param	bitLen　同期ビット数
 */
void F2FData_setSyncBitLength(int len)
{
	if (len < 0)
	{
		syncBitLength = 0;
	} else if (100 < len)
	{
		syncBitLength = 100;
	} else
	{
		syncBitLength = len;
	}
}


/**
 * F2Fエンコードする
 * @param	*data	データの先頭アドレス
 * @param	size	データ数
 * @param	bitLen	1データ当たりのビット数
 * @retval	総ビット数 (1clock周期につき2bit)
 */
int F2FData_encode(unsigned char* data, int size, int bitLen)
{
	int currLevel = 0;
	bitLength = 0;

	// 前端同期ビット
	currLevel = addSyncBit(currLevel);

	// データビット
	int i;
	for (i = 0; i < size; i++)
	{
		int bytePos;
		int bitPos;
		unsigned char c = data[i];

		int j;
		for (j = 0; j < bitLen; j++)
		{
			(currLevel == 0)? (currLevel = 1) : (currLevel = 0);

			bytePos = bitLength / 8;
			bitPos = bitLength % 8;
			dataBuff[bytePos] |= currLevel << bitPos;
			bitLength++;

			if (c & 0x01)
			{
				(currLevel == 0)? (currLevel = 1) : (currLevel = 0);
			}

			bytePos = bitLength / 8;
			bitPos = bitLength % 8;
			dataBuff[bytePos] |= currLevel << bitPos;
			bitLength++;

			c >>= 1;
		}
	}

	// 後端同期ビット
	addSyncBit(currLevel);

	return bitLength;
}


/**
 * F2Fデータバッファの先頭アドレスを取得する
 */
unsigned char* F2FData_getDataBuffAddr(void)
{
	return dataBuff;
}


/**
 * F2Fデータのビット数を取得する
 */
int F2FData_getBitLength(void)
{
	return bitLength;
}


/**
 * Syncビットを付加する
 * @param	currLevel	現在のHi/Low Level
 * @retval	最後のHi/Low Level
 */
int addSyncBit(int currLevel)
{
	int i;
	for (i = 0; i < syncBitLength; i++)
	{
		int bytePos;
		int bitPos;
		(currLevel == 0) ? (currLevel = 1) : (currLevel = 0);
		bytePos = bitLength / 8;
		bitPos = bitLength % 8;
		dataBuff[bytePos] |= currLevel << bitPos;
		bitLength++;

		bytePos = bitLength / 8;
		bitPos = bitLength % 8;
		dataBuff[bytePos] |= currLevel << bitPos;
		bitLength++;
	}
	return currLevel;
}
