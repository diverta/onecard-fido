#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsU2F.h"
#include "BleToolsUtil.h"

//
// ��Ɨ̈�
//
static unsigned char requestBuf[256];
static unsigned char responseBuf[1024];
static unsigned char encodedResponse[1024];

//
// BLE�f�o�C�X����̃��X�|���X��ێ�
//
static unsigned char reply[1024];
static unsigned char replyCmd;
static size_t        replyLength;
static uint16_t      replyStatusWord;

static void xferBleU2fRequest(pBleDevice dev)
{
	// ���b�Z�[�W�E�w�b�_�[����A�o�C�g�z��̐������������擾�iAPDU���{�R�j

	// �R�}���h�̓w�b�_�[�� 1�o�C�g��
	unsigned char CMD = requestBuf[0];

	// APDU���̓w�b�_�[�� 2, 3�o�C�g��
	size_t apduLen = requestBuf[1] * 256 + requestBuf[2];

	// APDU�͓]���o�C�g�z��� 4�o�C�g�ڈȍ~
	unsigned char *apduBuf = &requestBuf[3];

	// ���X�|���X����ݒ�
	replyLength = sizeof(reply);

	// ���O�o��
	// BleToolsUtil_outputLog("xferBleU2fRequest: APDU to BLE");
	// BleToolsUtil_outputDumpLog(apduBuf, apduLen);

	// BLE���N�G�X�g�𑗐M ---> ���X�|���X����M
	memset(reply, 0x00, sizeof(reply));
	ReturnValue retval = dev->CommandWrite(
		CMD, apduBuf, apduLen, &replyCmd, reply, &replyLength);

	// ��M�Ɏ��s�����ꍇ
	if (retval != ReturnValue::BLEAPI_ERROR_SUCCESS) {
		// ERROR�t���[���𐶐�
		//  CMD:   0xbf (ERROR)
		//  VALUE: 0x7f (ERR_OTHER)
		responseBuf[0] = 0xbf;
		responseBuf[1] = 0;
		responseBuf[2] = 1;
		responseBuf[3] = 0x7f;

		// ���O�o��
		BleToolsUtil_outputLog("xferBleU2fRequest: Command write failed");
		return;
	}

	// ���O�o��
	// BleToolsUtil_outputLog("xferBleU2fRequest: APDU from BLE");
	// BleToolsUtil_outputDumpLog(reply, replyLength);

	// ���X�|���X�f�[�^�Ƀw�b�_�[�AAPDU�̏��ŃR�s�[
	responseBuf[0] = replyCmd;
	responseBuf[1] = (unsigned char)(replyLength / 256);
	responseBuf[2] = replyLength % 256;
	for (size_t i = 0; i < replyLength; i++) {
		responseBuf[3 + i] = reply[i];
	}
}

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

	// �擾�������b�Z�[�W��BLE�փ��N�G�X�g
	//  BLE����̃��X�|���X�f�[�^�́A
	//  �w�b�_�[��APDU�𓯈�̃o�C�g�z��Ɋi�[
	xferBleU2fRequest(dev);

	// ���b�Z�[�W�E�w�b�_�[����A�o�C�g�z��̐������������擾�iAPDU���{�R�j
	int responseLen = responseBuf[1] * 256 + responseBuf[2] + 3;

	// ���O�o��
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage: Message back to U2F Helper");
	BleToolsUtil_outputDumpLog(responseBuf, responseLen);

	// ���X�|���X�f�[�^�̃o�C�g�z���
	// web-save base64������ɕϊ����AU2F Helper�ɓ]��
	if (BleToolsUtil_base64Encode((char *)responseBuf, responseLen, encodedResponse) > 0) {
		std::cout << encodedResponse << std::endl;
	}

	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage end");
	return true;
}
