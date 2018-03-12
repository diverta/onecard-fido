// U2FMaintenanceToolCMD.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

extern "C"
{
	__declspec(dllexport) int __stdcall ToolCMD_Hello(char* psz, int len)
	{
		// U2F管理コマンドライブラリーがロードできるかどうかテスト
		const char *sample = "U2F管理コマンドライブラリーをロードしました";
		strcpy_s(psz, len, sample);
		return strlen(sample);
	}
}
