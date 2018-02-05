﻿#include <stdio.h>

const char *TOOL_LOGFILE_NAME = "U2FMaintenanceTool.txt";

static inline int convertBase64CharTo6bitValue(int c)
{
	// base64の1文字を6bitの値に変換する
	if (c == '/' || c == '_')
		return 63;
	if (c == '+' || c == '-')
		return 62;
	if ('0' <= c && c <= '9')
		return (c - '0') + 52;
	if ('a' <= c && c <= 'z')
		return (c - 'a') + 26;
	if ('A' <= c && c <= 'Z')
		return (c - 'A');

	return 0;
}

int BleToolsUtil_base64Decode(const char* src, size_t src_len, unsigned char* dest)
{
	// base64の文字列srcをデコードしてdestに格納
	unsigned char  o0, o1, o2, o3;
	unsigned char *p = dest;
	for (size_t n = 0; n < src_len; n += 4) {
		o0 = convertBase64CharTo6bitValue(src[n]);
		o1 = convertBase64CharTo6bitValue(src[n + 1]);
		o2 = convertBase64CharTo6bitValue(src[n + 2]);
		o3 = convertBase64CharTo6bitValue(src[n + 3]);

		*p++ = (o0 << 2) | ((o1 & 0x30) >> 4);
		*p++ = ((o1 & 0xf) << 4) | ((o2 & 0x3c) >> 2);
		*p++ = ((o2 & 0x3) << 6) | o3 & 0x3f;
	}
	*p = 0;

	// 変換後のバイト数を返す
	return int(p - dest);
}

//
// For websafe B64 encoding
//
#define BINARY_UNIT_SIZE 3
#define BASE64_UNIT_SIZE 4

int BleToolsUtil_base64Encode(const char* src, size_t src_len, unsigned char* dest)
{
	// 文字列srcをbase64エンコードしてdestに格納
	static char base64_digits[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	const unsigned char *inputBuffer = (const unsigned char *)src;

	char *outputBuffer = (char *)dest;
	size_t outputBufferSize =
		((src_len / BINARY_UNIT_SIZE)
			+ ((src_len % BINARY_UNIT_SIZE) ? 1 : 0))
		* BASE64_UNIT_SIZE;
	outputBufferSize += 1;

	size_t i = 0;
	size_t j = 0;
	for (; i + BINARY_UNIT_SIZE - 1 < src_len; i += BINARY_UNIT_SIZE) {
		//
		// Inner loop: turn 48 bytes into 64 base64 characters
		//
		outputBuffer[j++] = base64_digits[(inputBuffer[i] & 0xFC) >> 2];
		outputBuffer[j++] = base64_digits[((inputBuffer[i] & 0x03) << 4)
			| ((inputBuffer[i + 1] & 0xF0) >> 4)];
		outputBuffer[j++] = base64_digits[((inputBuffer[i + 1] & 0x0F) << 2)
			| ((inputBuffer[i + 2] & 0xC0) >> 6)];
		outputBuffer[j++] = base64_digits[inputBuffer[i + 2] & 0x3F];
	}

	if (i + 1 < src_len)
	{
		//
		// Handle the single '=' case
		//
		outputBuffer[j++] = base64_digits[(inputBuffer[i] & 0xFC) >> 2];
		outputBuffer[j++] = base64_digits[((inputBuffer[i] & 0x03) << 4)
			| ((inputBuffer[i + 1] & 0xF0) >> 4)];
		outputBuffer[j++] = base64_digits[(inputBuffer[i + 1] & 0x0F) << 2];
		outputBuffer[j++] = '=';

	} else if (i < src_len) {
		//
		// Handle the double '=' case
		//
		outputBuffer[j++] = base64_digits[(inputBuffer[i] & 0xFC) >> 2];
		outputBuffer[j++] = base64_digits[(inputBuffer[i] & 0x03) << 4];
		outputBuffer[j++] = '=';
		outputBuffer[j++] = '=';
	}
	outputBuffer[j] = 0;

	// 変換後のバイト数を返す
	return j;
}

//
// for debug use
//
void BleToolsUtil_outputLog(const char *msg)
{
	FILE *file_;
	fopen_s(&file_, TOOL_LOGFILE_NAME, "a");
	fprintf(file_, "%s\n", msg);
	fclose(file_);
}

void BleToolsUtil_outputDumpLog(unsigned char *bin, size_t length)
{
	FILE *file_;
	fopen_s(&file_, TOOL_LOGFILE_NAME, "a");
	for (size_t i = 0; i < length; i++) {
		fprintf(file_, "%02x ", bin[i]);
		if (i % 16 == 15 && i < length - 1) {
			fprintf(file_, "\n");
		}
	}
	fprintf(file_, "\n");
	fclose(file_);
}
