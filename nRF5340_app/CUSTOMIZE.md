# カスタマイズ内容

USB経由によるファームウェア更新（DFU）に対応させるため、Zephyrプラットフォームのコードを直接カスタマイズしています。

## Zephyrライブラリーのカスタマイズ

以下のファイルは、`secure_device_app`のプロジェクトファイル（`CMakeLists.txt`等）で制御できないため、直接内容を修正する必要があります。

#### MCUBOOTのカスタマイズ

ディレクトリー＝`ncs/bootloader/mcuboot/boot/zephyr`

|ファイル名|変更内容|変更目的|備考|
|:--|:-|:-|:-|
|`prj.conf`|定義項目修正／追加|ログ化け回避、ブートローダー待機機能の有効化||
|`main.c`|プログラムコード部分修正|汎用リテンション・レジスターによるブートローダー動作制御||
|`Kconfig`|定義項目追加|USB DFU I/F導入によるプログラムサイズ拡張||


#### USB I/Fのカスタマイズ

ディレクトリー＝`ncs/zephyr/subsys/usb/class/dfu`

|ファイル名|変更内容|変更目的|備考|
|:--|:-|:-|:-|
|`usb_dfu.c`|プログラムコード部分削除|ファームウェアイメージを抜き取られるのを回避||

## 個別修正内容

##### ncs/bootloader/mcuboot/boot/zephyr/prj.conf

- 詳細ログ出力無効化（ログ化けの発生回避）
- ブートローダー待機機能の有効化（同時に、USB DFU I/Fが組み込まれます）

```
bash-3.2$ diff ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/bootloader/mcuboot/boot/zephyr/prj.conf.original ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/bootloader/mcuboot/boot/zephyr/prj.conf
33c33
< CONFIG_LOG=y
---
> CONFIG_LOG=n
38a39,40
>
> CONFIG_BOOT_USB_DFU_WAIT=y
bash-3.2$
```

##### ncs/bootloader/mcuboot/boot/zephyr/main.c

- 汎用リテンション・レジスターによるブートローダー動作制御<br>
`nrf_power_gpregret_get`で取得した値が`0xb1`の場合、ブートローダーモードに遷移します。<br>
すなわち、すぐにファームウェアをブートするのではなく、USB DFU I/F経由でDFUコマンド／ファームウェア更新イメージが受信されるまで、`CONFIG_BOOT_USB_DFU_WAIT_DELAY_MS`で指定された時間（最大１２秒間）待機します。

```
bash-3.2$ diff ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/bootloader/mcuboot/boot/zephyr/main.c.original ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/bootloader/mcuboot/boot/zephyr/main.c
541,543c541,552
<         BOOT_LOG_INF("Waiting for USB DFU");
<         wait_for_usb_dfu(K_MSEC(CONFIG_BOOT_USB_DFU_WAIT_DELAY_MS));
<         BOOT_LOG_INF("USB DFU wait time elapsed");
---
>         uint8_t gp_reg = nrf_power_gpregret_get(NRF_POWER);
>         if (gp_reg == 0xb1) {
>             // Bootloader should enter DFU mode
>             //   0xb1: BOOTLOADER_DFU_START
>             //   (BOOTLOADER_DFU_GPREGRET | BOOTLOADER_DFU_START_BIT_MASK)
>             nrf_power_gpregret_set(NRF_POWER, 0x00);
>             BOOT_LOG_INF("Waiting for USB DFU");
>             wait_for_usb_dfu(K_MSEC(CONFIG_BOOT_USB_DFU_WAIT_DELAY_MS));
>             BOOT_LOG_INF("USB DFU wait time elapsed");
>         } else {
>             BOOT_LOG_INF("Ignoring gpregret value: 0x%02x", gp_reg);
>         }
bash-3.2$
```

##### ncs/bootloader/mcuboot/boot/zephyr/Kconfig

- USB DFU I/F導入によるプログラムサイズ拡張<br>
ブートローダーのプログラムサイズを、`0x2000`（16,384バイト）拡張しています。

```
bash-3.2$ diff ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/bootloader/mcuboot/boot/zephyr/Kconfig.original ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/bootloader/mcuboot/boot/zephyr/Kconfig
44c44
< 	default 0xc000
---
> 	default 0x10000
bash-3.2$
```
