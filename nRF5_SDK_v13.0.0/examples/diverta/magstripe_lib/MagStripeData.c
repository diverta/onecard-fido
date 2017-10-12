/*
 * MagStripeData.c
 *
 *  Created on: 2014/11/10
 *      Author: shoji
 */

#include "MagStripeData.h"
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

static const int maxDataLength = MAX_DATA_LENGTH;
STATIC int bitLength;
static TrackNumber TrackNum;
STATIC unsigned char Data[MAX_DATA_LENGTH + 1];
STATIC unsigned char EncodedData[MAX_DATA_LENGTH + 2];
STATIC int DataLength;
STATIC int EncodedDataLength;


/*
 * Private Function
 */
void addTrack1(char c);
void addTrack2_3(char c);
void addJIS2(char c);
void addOddParity(void);
void addEvenParity(void);
void addLRC(void);
void addLRC_JIS2(void);
char oddParity(char c);
char evenParity(char c);


/**
 * 初期化
 */
void MagStripeData_init(void)
{
	F2FData_init();

	int i;
	for (i = 0; i < maxDataLength; i++)
	{
		Data[i] = 0;
		EncodedData[i] = 0;
	}
	EncodedData[i] = 0;
	DataLength = 0;
	EncodedDataLength = 0;
	TrackNum = TRACK_2;
	bitLength = 5;
}


/**
 * 対象とするTrack番号を設定する
 * @param	tn	Track番号
 */
void MagStripeData_setTrackNumber(TrackNumber tn)
{
	TrackNum = tn;
	switch (TrackNum)
	{
	case TRACK_1:
		bitLength = 7;
		break;

	case JIS_2:
		bitLength = 8;
		break;

	case TRACK_2: /* FALLTROUGH */
	case TRACK_3:
		bitLength = 5;
		break;

	default:
		/* ここには来ない */
		break;
	}
}


/**
 * 対象としているトラック番号を取得する
 * @retval	トラック番号
 */
TrackNumber MagStripeData_getTrackNumber(void)
{
	return TrackNum;
}


/**
 * 同期ビット数を設定する
 * @param	len		同期ビット数
 */
void MagStripeData_setSyncBitLength(int len)
{
	F2FData_setSyncBitLength(len);
}


/**
 * データクリア
 */
void MagStripeData_clear(void)
{
	int i;
	for (i = 0; i < maxDataLength; i++)
	{
		Data[i] = 0;
		EncodedData[i] = 0;
	}
	EncodedData[i] = 0;
	DataLength = 0;

	F2FData_clear();
}


/**
 * 1文字追加
 *
 * TrackNumberによって有効な文字が異なる
 * Track1: 20H(スペース)～5FH(_)
 *         61H(a)～7AH(z) は大文字に変換する
 * Track2,3: 30H(0)～3FH(?)
 * JIS 2: 7bit ASCII
 * トラック毎の最大文字数は考慮しない
 */
void MagStripeData_add(char c)
{
	if (DataLength < maxDataLength)
	{
		switch (TrackNum)
		{
		case TRACK_1:
			addTrack1(c);
			break;

		case TRACK_2: /* FALLTHROUGH */
		case TRACK_3:
			addTrack2_3(c);
			break;

		case JIS_2:
			if (DataLength == 0) {	// 先頭に開始コード(0x7F)追加
				addJIS2(0x7F);
			}
			addJIS2(c);
			break;

		default:
			/* ここには来ない */
			break;
		}
	}
}


/**
 * データエンコード
 * パリティビットとLRCを付加する
 */
void MagStripeData_encode(void)
{
	switch (TrackNum)
	{
	case TRACK_1:	/* FALLTHROUGH */
	case TRACK_2:	/* FALLTHROUGH */
	case TRACK_3:
		addOddParity();
		addLRC();
		F2FData_encode(EncodedData, DataLength, bitLength);
		break;

	case JIS_2:
		addJIS2(0x7F);		// 終了コード(0x7F)追加
		addEvenParity();
		addLRC_JIS2();
		F2FData_encode(EncodedData, DataLength, bitLength);
		break;

	default:
		/* ここには来ない */
		break;
	}
}


/**
 * F2Fデータバッファの先頭アドレスを取得する
 */
unsigned char* MagStripeData_getF2FDataAddr(void)
{
	return F2FData_getDataBuffAddr();
}


/**
 * F2Fデータのビット数を取得する
 */
int MagStripeData_getF2FDataBitLength(void)
{
	return F2FData_getBitLength();
}


/**
 * 文字追加(Track1)
 */
void addTrack1(char c)
{
	if (('a' <= c) && (c <= 'z'))
	{
		c -= 0x20;
	}
	else if ((c < ' ') || ('_' < c))
	{
		return;
	}

	Data[DataLength] = c;
	DataLength++;
}


/**
 * 文字追加(Track2,3)
 */
void addTrack2_3(char c)
{
	if (('0' <= c) && (c <= '?'))
	{
		Data[DataLength] = c;
		DataLength++;
	}
}


/**
 * 文字追加(JIS 2)
 */
void addJIS2(char c)
{
	Data[DataLength] = c;
	DataLength++;
}


/**
 * 奇数パリティ付加
 */
void addOddParity(void)
{
	char offset = 0;

	switch (TrackNum)
	{
	case TRACK_1:
		offset = 0x20;
		break;

	case TRACK_2: /* FALL THROUGH */
	case TRACK_3:
		offset = 0x30;
		break;

	case JIS_2:
		// Do Nothing
		break;

	default:
		/* ここには来ない */
		break;
	}

	int pos;
	for (pos = 0; pos < DataLength; pos++)
	{
		EncodedData[pos] = oddParity(Data[pos] - offset);
	}
}


/**
 * 偶数パリティ追加
 */
void addEvenParity(void)
{
	if (TrackNum == JIS_2)
	{
		int pos;
		for (pos = 0; pos < DataLength; pos++)
		{
			EncodedData[pos] = evenParity(Data[pos]);
		}
	}
}


/**
 * LRC付加 (Track1,2,3)
 */
void addLRC(void)
{
	char c = 0;
	int pos;

	for (pos = 0; pos < DataLength; pos++)
	{
		c ^= EncodedData[pos];
	}
	EncodedData[pos] = oddParity(c);
	DataLength++;
}


/**
 * LRC付加 (JIS-II)
 */
void addLRC_JIS2(void)
{
	char c = 0;
	int pos;

	for (pos = 1; pos < DataLength; pos++)
	{
		c ^= EncodedData[pos];
	}
	EncodedData[pos] = evenParity(c);
	DataLength++;
}


/**
 * 奇数パリティビット付加
 * @param c
 * @retval cにパリティビットを付加したデータ
 */
char oddParity(char c)
{
	unsigned char uc = (unsigned char) c;
	unsigned int cnt = 0;
	int i;
	for (i = 0; i < bitLength - 1; i++)
	{
		cnt += uc & 0x01;
		uc >>= 1;
	}
	cnt = (cnt + 1) & 0x01;
	return c | (cnt << (bitLength - 1));
}


/**
 * 偶数パリティビット追加
 * @param	c
 * @retval	cにパリティビットを付加したデータ
 */
char evenParity(char c)
{
	unsigned char uc = (unsigned char) c;
	unsigned int cnt = 0;
	int i;
	for (i = 0; i < bitLength - 1; i++)
	{
		cnt += uc & 0x01;
		uc >>= 1;
	}
	cnt = cnt & 0x01;
	return c | (cnt << (bitLength - 1));
}
