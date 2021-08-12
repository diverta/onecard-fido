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

##### ncs/zephyr/subsys/usb/class/dfu/usb_dfu.c

- ファームウェアイメージを抜き取られるのを回避<br>
`dfu-util`の`upload`コマンド実行により、ファームウェアイメージ（定数等を含むプログラム部分）をPCなどに容易にエクスポートできてしまう脆弱性があります。<br>
このため、nRF5340側のブートローダーで`upload`コマンドを実行できないよう、該当処理のコードを削除しています。

```
bash-3.2$ diff ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/zephyr/subsys/usb/class/dfu/usb_dfu.c.original ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/zephyr/subsys/usb/class/dfu/usb_dfu.c
499,582d498
< 	case DFU_UPLOAD:
< 		LOG_DBG("DFU_UPLOAD block %d, len %d, state %d",
< 			pSetup->wValue, pSetup->wLength, dfu_data.state);
<
< 		if (dfu_check_app_state()) {
< 			return -EINVAL;
< 		}
<
< 		switch (dfu_data.state) {
< 		case dfuIDLE:
< 			dfu_reset_counters();
< 			LOG_DBG("DFU_UPLOAD start");
< 		case dfuUPLOAD_IDLE:
< 			if (!pSetup->wLength ||
< 			    dfu_data.block_nr != pSetup->wValue) {
< 				LOG_DBG("DFU_UPLOAD block %d, expected %d, "
< 					"len %d", pSetup->wValue,
< 					dfu_data.block_nr, pSetup->wLength);
< 				dfu_data.state = dfuERROR;
< 				dfu_data.status = errUNKNOWN;
< 				break;
< 			}
<
< 			/* Upload in progress */
< 			bytes_left = dfu_data.flash_upload_size -
< 				     dfu_data.bytes_sent;
< 			if (bytes_left < pSetup->wLength) {
< 				len = bytes_left;
< 			} else {
< 				len = pSetup->wLength;
< 			}
<
< 			if (len > USB_DFU_MAX_XFER_SIZE) {
< 				/*
< 				 * The host could requests more data as stated
< 				 * in wTransferSize. Limit upload length to the
< 				 * size of the request-buffer.
< 				 */
< 				len = USB_DFU_MAX_XFER_SIZE;
< 			}
<
< 			if (len) {
< 				const struct flash_area *fa;
<
< 				ret = flash_area_open(dfu_data.flash_area_id,
< 						      &fa);
< 				if (ret) {
< 					dfu_data.state = dfuERROR;
< 					dfu_data.status = errFILE;
< 					break;
< 				}
< 				ret = flash_area_read(fa, dfu_data.bytes_sent,
< 						      *data, len);
< 				flash_area_close(fa);
< 				if (ret) {
< 					dfu_data.state = dfuERROR;
< 					dfu_data.status = errFILE;
< 					break;
< 				}
< 			}
< 			*data_len = len;
<
< 			dfu_data.bytes_sent += len;
< 			dfu_data.block_nr++;
<
< 			if (dfu_data.bytes_sent == dfu_data.flash_upload_size &&
< 			    len < pSetup->wLength) {
< 				/* Upload completed when a
< 				 * short packet is received
< 				 */
< 				*data_len = 0;
< 				dfu_data.state = dfuIDLE;
< 			} else
< 				dfu_data.state = dfuUPLOAD_IDLE;
<
< 			break;
< 		default:
< 			LOG_ERR("DFU_UPLOAD wrong state %d", dfu_data.state);
< 			dfu_data.state = dfuERROR;
< 			dfu_data.status = errUNKNOWN;
< 			dfu_reset_counters();
< 			return -EINVAL;
< 		}
< 		break;
bash-3.2$
```
