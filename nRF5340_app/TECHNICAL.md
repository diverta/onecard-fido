# 技術情報補足

最新更新日：2022/3/24

## 概要

nRF5340や、Zephyrプラットフォームに関する技術情報を、補足的に掲載しています。

## Zephyr技術情報

Zephyrプラットフォームに関する技術情報になります。

- USBシリアル番号について

### USBシリアル番号について

Zephyrでは、USBデバイスのシリアル番号を、ハードウェアIDから取得して生成しています。<br>
その詳細について掲載いたします。

#### ハードウェアIDについて

最終更新日現在、nRF52、nRF53シリーズでは、ハードウェアIDは、レジスター`FICR.DEVICEID`に格納されているようです。[注1]

`DEVICEID`は、64ビット（8バイト）のレジスターです。<br>
これを、Zephyrでは`nrf_ficr_deviceid_get`というAPIを直接利用し取得しています。

```
//
// ncs/zephyr/drivers/hwinfo/hwinfo_nrf.c
//
ssize_t z_impl_hwinfo_get_device_id(uint8_t *buffer, size_t length)
{
    struct nrf_uid dev_id;

    dev_id.id[0] = sys_cpu_to_be32(nrf_ficr_deviceid_get(NRF_FICR, 1));
    dev_id.id[1] = sys_cpu_to_be32(nrf_ficr_deviceid_get(NRF_FICR, 0));

    if (length > sizeof(dev_id.id)) {
        length = sizeof(dev_id.id);
    }

    memcpy(buffer, dev_id.id, length);

    return length;
}
```

Zephyr USBサブシステムでは、下記関数`usb_update_sn_string_descriptor`により、上記で取得した`DEVICEID`のうち、`CONFIG_USB_DEVICE_SN`（ユーザー定義）で指定した文字列の長さを上限として、HEX文字列を生成します。

```
//
// ncs/zephyr/subsys/usb/usb_descriptor.c
//
/*
 * Default USB Serial Number string descriptor will be derived from
 * Hardware Information Driver (HWINFO). User can implement own variant
 * of this function. Please note that the length of the new Serial Number
 * descriptor may not exceed the length of the CONFIG_USB_DEVICE_SN. In
 * case the device ID returned by the HWINFO driver is bigger, the lower
 * part is used for the USB Serial Number, as that part is usually having
 * more entropy.
 */
__weak uint8_t *usb_update_sn_string_descriptor(void)
{
    /*
     * The biggest device ID supported by the HWINFO driver is currently
     * 128 bits, which is 16 bytes. Assume this is the maximum for now,
     * unless the user requested a longer serial number.
     */
    const int usblen = sizeof(CONFIG_USB_DEVICE_SN) / 2;
    uint8_t hwid[MAX(16, sizeof(CONFIG_USB_DEVICE_SN) / 2)];
    static uint8_t sn[sizeof(CONFIG_USB_DEVICE_SN) + 1];
    const char hex[] = "0123456789ABCDEF";
    int hwlen, skip;

    memset(hwid, 0, sizeof(hwid));
    memset(sn, 0, sizeof(sn));

    hwlen = hwinfo_get_device_id(hwid, sizeof(hwid));
    if (hwlen > 0) {
        skip = MAX(0, hwlen - usblen);
        LOG_HEXDUMP_DBG(&hwid[skip], usblen, "Serial Number");
        for (int i = 0; i < usblen; i++) {
            sn[i * 2] = hex[hwid[i + skip] >> 4];
            sn[i * 2 + 1] = hex[hwid[i + skip] & 0xF];
        }
    }

    return sn;
}
```

Zephyr USBサブシステムの初期化処理においては、前述のHEX文字列が、USBデバイスのシリアル番号として使用されます。

下記コードは、生成されたUSBデバイスのシリアル番号を確認する例です。

```
//
// onecard-fido/nRF5340_app/secure_device_app/prj.conf
//
#
# for USB
#
CONFIG_USB_DEVICE_SN="000000000000"

//
// onecard-fido/nRF5340_app/secure_device_app/src/app_usb_hid.c
//
#include <drivers/hwinfo.h>
void app_usb_hid_configured(const uint8_t *param)
{
    // 内部変数をクリア
    (void)param;
    memset(m_report, 0, sizeof(m_report));

    // for research
    uint8_t *p = usb_update_sn_string_descriptor();
    LOG_ERR("app_usb_hid_configured %s", log_strdup(p));
}
```

下記は確認結果になります。<br>
前述の通り、`CONFIG_USB_DEVICE_SN`で指定した文字数（12文字＝6バイト分）のシリアル番号`B4A2039693C9`が、HEX文字列により得られていることが確認できます。

```
*** Booting Zephyr OS build v2.7.99-ncs1-1  ***
[00:00:00.014,160] <inf> app_crypto: Mbed TLS random seed initialized
[00:00:00.014,404] <inf> app_usb: USB initialized
：
[00:00:00.191,833] <inf> usb_hid: Device configured
[00:00:00.191,894] <err> app_usb_hid: app_usb_hid_configured B4A2039693C9
```

[注1] FICR=Factory information configuration registers。DEVICEIDの仕様は<b>[こちらのドキュメント](https://infocenter.nordicsemi.com/topic/ps_nrf5340/chapters/ficr/doc/ficr.html?cp=3_0_0_4_3_1#register.INFO.DEVICEID)</b>に記載されています。
