/* 
 * File:   ccid_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/06/02, 11:06
 */
#include <stdlib.h>
#include <string.h>

#include "ccid_piv.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// テスト用の仮データ
//  正式には、yubico-piv-tool 等を利用し、
//  Flash ROMにインストールした証明書等データを
//  使用することになります。
//
#define CCID_PIV_OBJECT_TEST true
#if CCID_PIV_OBJECT_TEST
static char *SN_TEMP    = "7BEEBABE";
static char *CHUID_TEMP = "533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34104C8D536A86AA98A5CE20D53557776E58350832303330303130313E00FE00";
static char *CCC_TEMP   = "5333f015a000000116ff02d4bfab488d66fa69ae507ee5f8daf10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00";
static char *TAG05_TEMP = "538203217082031830820314308201FCA003020102020900AE820BAEB41C48A0300D06092A864886F70D01010B0500301F311D301B06035504030C14706976617574682E6469766572742E636F2E6A70301E170D3230303531383036303635355A170D3231303531383036303635355A301F311D301B06035504030C14706976617574682E6469766572742E636F2E6A7030820122300D06092A864886F70D01010105000382010F003082010A0282010100AD9651A5E30EED05E465A68917B578C883D9F203AB443981327F11E2878FFD55D25DF723EFD0F969C72243C7B0B0E518994DE392D877AFC95ADFAAECE057AF3038F07B8405E4144CEF6D911F752B607710305730C1BE44BBEBE79BB2FD004E7DBAB899905F19AA3CB771453A24934B0AE3496B42740F4CD315A8EC3BFDAEB781FA0C462B25B3E8B7AD01E8F90457268987A92143A0B63F18DABCDF5356FC1F3A6D7E425C22F4011946049DB125BA3883EAE85CED71908DF04B3F67D5C794470278485E4CB9363F4DF2ECE77CB66766E2A4935BBC099EA6103F99C538691F6EE796E2F64FCAAC2A3F0111A7B47BA8A15FA82562F4B63D85A97E6DF9B949F042E10203010001A3533051301D0603551D0E04160414FBEA9655E126927DA296075E172152749AB14348301F0603551D23041830168014FBEA9655E126927DA296075E172152749AB14348300F0603551D130101FF040530030101FF300D06092A864886F70D01010B05000382010100517BF961E1BCBEFAFD677DF8E9165C5BBA693C580FB3DC4109060A48D21DB72C6852222B609557047FBD567AF0BE503BE4AEB6DB363ADC9906EFD7A96BC35579055F23DC36F1F1FB28E965972C3E5F0F7381E2A99F13BD66C72A4AB1760E7CD160E2B144D6631AA5D426C83BAD959A10451947D233B035BAE80D7F9EA39C152611A2C22BBC0BC21A2484316DD8EAC8BD5CBBF943D6DF1CC5B91BC47A20FE6946AB443357583C73D3782B151C71B5699451096CE6BF26DFBC05CBC957EA0DC9870AA7F01F0AA10C86ACF441AEE4C4A58E84715AEE8074066CC2BF466E151B5216A64409ED36543FEF0855E600ADEF14C55DD4B651605C403B72FE8F276C3291AE710100FE00";
static char *TAG0A_TEMP = "538203217082031830820314308201FCA003020102020900A88E831401D3D661300D06092A864886F70D01010B0500301F311D301B06035504030C146469677369676E2E6469766572742E636F2E6A70301E170D3230303531383036303735365A170D3231303531383036303735365A301F311D301B06035504030C146469677369676E2E6469766572742E636F2E6A7030820122300D06092A864886F70D01010105000382010F003082010A0282010100B2EC485D2B0A79EB4EA8333A26445606D35A50B21BAB95B8B80BA4BDAEC24F200785A64E489E5EE65516FCE942D6AD78257944124FBFE0E4E02FC29DC2E433837A41C1BCAA686286D3DA7D2D5EBDDAE9EEC51444E2CADB8D1A7CBC40DC3C64A79E41F94D51F73C9AC2240B2FCD01DAB02A95091C787E2E10E75C2B3BB34FB957E68214B99EF38F75CD2081C572BBA84B81B1CD9F282D3DA394068ECD8961777F16726BDFDC1CB77F26ED1953D8D49E841C209879FE3DB7C7D71CAE2BE38CFCF1665FF3F1503A79081178084D57DFC4BFBFD1FD0DB14528AE2A0D4024302209A6DD8DEC19F6BDA6B03BCA80E5403A460C92620ECC2561B55695E2BA6C9AE5587B0203010001A3533051301D0603551D0E041604145C9D2D82687909E84E8FE1F91EF2EF1BB58FD6C3301F0603551D230418301680145C9D2D82687909E84E8FE1F91EF2EF1BB58FD6C3300F0603551D130101FF040530030101FF300D06092A864886F70D01010B0500038201010091E1F97A48347AF72076D15AF353C24B85B2ECD44EB188688330824E47295320EC7CD6063E2E282510AD058BC0BE036F6D5AD2476538F92353507183A0DEE96B89D29ACAA84C8A96CD85D4760C74D1EC315B26ACA0D142D8A1DDE82654A636787728583E6AC1094E4AC9544003AF70DD7F9D317B1C1C45A25807ABDEA32775990B532BA20F1C4376101C08BE555098F6FD184DAF6D8D14BD276305A947DBE2228FB1CDA1825EA5BFACBE44FAC657D6A90BD2AC4F0761029518B2704FD24701AF7E0E7F20194FF71F56C140C6B1037C373E6BCA1F8A539DA0221A150130FF2C149F7500A99636F4AB8FA5339332AB6EFDAD2B240CBB56101E20CD0FD5804476E0710100FE00";
static char *TAG0B_TEMP = "538203217082031830820314308201FCA00302010202090091DA3584A0BBA985300D06092A864886F70D01010B0500301F311D301B06035504030C146B65796D676D742E6469766572742E636F2E6A70301E170D3230303532303036313533375A170D3231303532303036313533375A301F311D301B06035504030C146B65796D676D742E6469766572742E636F2E6A7030820122300D06092A864886F70D01010105000382010F003082010A0282010100B0ACE4165F3820E343982B32450583F70F4BB8B77115DAB86BF29A7F826E8E5275E847215F7233292253F38D6559A6FCEDA203D244EA42A40F1AA93B3D7F352E120BD8F12FA7D0EB037FC3212A9C1F657CB49D1DA1EA98A94FDF643D25EAD466EA72300DB0ABF70F1147EA0FB737B3235E7BC8F2ABEF5C32BD21ACD902E526AA5B45103FFEF99CBE0A1386FE244AF91974474D132D0FCBD1742499AFE7A475EBF998E52FA69F6A972F357BE062B2DFFFF29BA845FC39F7C1FE96736EA3053290360F3CD3FF0E46C76C9A64B67463CDE7FFF973190E851CD551352B10FDFE69B1A62BF84AA8C12A447EF2C5B4F06F9E0A9B2F12CE749A89F402E7AFEA34FBA7090203010001A3533051301D0603551D0E041604143D005264D7F7BCEBCE29B32DF5534FCB632F7638301F0603551D230418301680143D005264D7F7BCEBCE29B32DF5534FCB632F7638300F0603551D130101FF040530030101FF300D06092A864886F70D01010B050003820101001E86C045A129FE22BECC33B652E5E3480333938FB7EDC8B206FCA5B33508ACE1422F409469772C2B30AF05A105ED28672B03CEE5345F5900421F93C3DC28756380B21C54361EDA0AC892D24245CEAFA393E5D8B5DC8E46EF5FC640BF13B1F3C46DC43ACD748BE3ED7FA0285C7484AFDA8D8F4982C26BAA45ACC43663FAA06582E3F46B8BF4AE603957C1304BB120F23B80924E993F7A4149B575129182801CC53F895144DFE5CC3AD744967D389CC236280FB49B8712303848B7566A71BEB40BAE6BB061A9B3F791463AC753C9953AE1473B5D450C167585164CB16605153C155687669F0606145B7BE2515B6DC60CF3D82821C1F606606127B4F8E124A4FAA6710100FE00";
#endif

// デフォルトの管理用キー (24 bytes)
static char *card_admin_key_default = "010203040506070801020304050607080102030405060708";

static size_t convert_hexstring_to_bytes(char *data, uint8_t *buffer)
{
    char buf[8];
    char *e;
    unsigned long value;
    size_t index = 0;
    size_t length = strlen(data);
    memset(buf, 0, sizeof(buf));

    for (int i = 0; i < length; i += 2) {
        // Hex文字列を２文字分抽出
        strncpy(buf, data + i, 2);
        // Hex文字列を数値に変換する
        value = strtoul(buf, &e, 16);
        if (*e != 0) { 
            // 変換失敗した場合は処理終了
            return 0;

        } else {
            buffer[index++] = (uint8_t)value;
        }
    }
    return index;
}

bool ccid_piv_object_sn_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(SN_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("Serial number is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(CHUID_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("Card Holder Unique Identifier is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(CCC_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("Card Capability Container is requested");
    return false;
#endif
}

bool ccid_piv_object_cert_cauth_get(uint8_t *buffer, size_t *size)
{
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Card Authentication is requested");
    return false;
}

bool ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG05_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for PIV Authentication is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG0A_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Digital Signature is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG0B_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Key Management is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_key_history_get(uint8_t *buffer, size_t *size)
{
    // 後日正式に実装予定です。
    fido_log_debug("Key History Object is requested");
    return false;
}

bool ccid_piv_object_card_admin_key_get(uint8_t *buffer, size_t *size)
{
    // デフォルトを戻す
    *size = convert_hexstring_to_bytes(card_admin_key_default, buffer);
    fido_log_debug("Card administration key is requested (%d bytes)", *size);
    return true;
}

uint8_t ccid_piv_object_card_admin_key_alg_get(void)
{
    // デフォルトを戻す
    // 0x03: 3-key triple DEA
    // 0x07: ALG_RSA_2048
    // 0x11: ALG_ECC_256
    return 0x03;
}
