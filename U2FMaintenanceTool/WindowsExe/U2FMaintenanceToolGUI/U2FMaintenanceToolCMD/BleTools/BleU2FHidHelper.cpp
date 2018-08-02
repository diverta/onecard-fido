#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsU2F.h"
#include "BleToolsUtil.h"

//
// ��Ɨ̈�
//
static unsigned char requestBuf[128];
static unsigned char responseBuf[1024];
static unsigned char encodedResponse[1024];

static int decodeWebsafeB64String(char *src, unsigned char *dest, size_t destLen)
{
	memset(dest, 0x00, destLen);
	int len = BleToolsUtil_base64Decode(src, strlen(src), dest);
	return len;
}

bool BleU2FHidHelper_ProcessXferMessage(char *recv_hid_message, pBleDevice dev)
{
	// U2F Helper�ƕW�����o�͂ɂ����Ƃ���s��
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage start");

	// �]�����ꂽ���b�Z�[�W���o�C�g�z��ɕϊ�
	decodeWebsafeB64String(recv_hid_message, requestBuf, sizeof(requestBuf));
	// ���b�Z�[�W�E�w�b�_�[����A�o�C�g�z��̐������������擾�iAPDU���{�R�j
	int requestLen = requestBuf[1] * 256 + requestBuf[2] + 3;

	// ���O�o��
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage: Message from U2F Helper");
	BleToolsUtil_outputDumpLog(requestBuf, requestLen);

	//
	// TODO �擾�������b�Z�[�W��BLE�փ��N�G�X�g
	//

	// 
	// TODO BLE����̃��X�|���X�f�[�^���o�C�g�z��ɓW�J
	//

	// ����͉��R�[�h�ł��B
	for (int i = 0; i < requestLen; i++) {
		responseBuf[i] = requestBuf[i];
	}
	// ���b�Z�[�W�E�w�b�_�[����A�o�C�g�z��̐������������擾�iAPDU���{�R�j
	int responseLen = responseBuf[1] * 256 + responseBuf[2] + 3;

	// ���O�o��
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage: Message back to U2F Helper");
	BleToolsUtil_outputDumpLog(responseBuf, responseLen);

	// ���X�|���X�f�[�^�̃o�C�g�z���
	// web-save base64������ɕϊ����AU2F Helper�ɓ]��
	if (BleToolsUtil_base64Encode((char *)responseBuf, requestLen, encodedResponse) > 0) {
		std::cout << encodedResponse << std::endl;
	}

	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage end");
	return true;
}
