#include <stdio.h>

const char *TOOL_LOGFILE_NAME = "U2FMaintenanceTool.txt";

static inline int convertBase64CharTo6bitValue(int c)
{
	// base64の1文字を6bitの値に変換する
	if (c == '/' || c == '-')
		return 63;
	if (c == '+' || c == '_')
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

int BleToolsUtil_base64Encode(const char* src, size_t src_len, unsigned char* dest)
{
	// 文字列srcをbase64エンコードしてdestに格納
	static char base64_digits[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

	unsigned char *p = dest;
	while (src_len > 0) {
		// read three source bytes (24 bits) 
		unsigned char s1 = src[0];
		unsigned char s2 = 0; if (src_len>1) s2 = src[1];
		unsigned char s3 = 0; if (src_len>2) s3 = src[2];

		unsigned int n;
		n  = s1;    // xxx1
		n <<= 8;    // xx1x
		n |= s2;    // xx12  
		n <<= 8;    // x12x
		n |= s3;    // x123  

		// get four 6-bit values for lookups
		unsigned char m4 = n & 0x3f;  n >>= 6;
		unsigned char m3 = n & 0x3f;  n >>= 6;
		unsigned char m2 = n & 0x3f;  n >>= 6;
		unsigned char m1 = n & 0x3f;

		// lookup the right digits for output
		unsigned char b1 = base64_digits[m1];
		unsigned char b2 = base64_digits[m2];
		unsigned char b3 = base64_digits[m3];
		unsigned char b4 = base64_digits[m4];

		// end of input handling
		*p++ = b1;
		*p++ = b2;
		if (src_len >= 3) {  // 24 src bits left to encode, output xxxx
			*p++ = b3;
			*p++ = b4;
		}
		if (src_len == 2) {  // 16 src bits left to encode, output xxx=
			*p++ = b3;
			*p++ = '=';
		}
		if (src_len == 1) {  // 8 src bits left to encode, output xx==
			*p++ = '=';
			*p++ = '=';
		}
		src += 3;
		src_len -= 3;
	}
	*p = 0x00;

	// 変換後のバイト数を返す
	return int(p - dest);
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
