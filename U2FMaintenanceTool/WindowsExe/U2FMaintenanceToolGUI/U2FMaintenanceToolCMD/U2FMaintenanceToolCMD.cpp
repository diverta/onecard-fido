// U2FMaintenanceToolCMD.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"

extern "C"
{
	__declspec(dllexport) int __stdcall ToolCMD_Hello(char* psz, int len)
	{
		// U2F�Ǘ��R�}���h���C�u�����[�����[�h�ł��邩�ǂ����e�X�g
		const char *sample = "U2F�Ǘ��R�}���h���C�u�����[�����[�h���܂���";
		strcpy_s(psz, len, sample);
		return strlen(sample);
	}
}
