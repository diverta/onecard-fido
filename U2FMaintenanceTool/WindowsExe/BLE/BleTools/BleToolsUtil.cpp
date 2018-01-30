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
