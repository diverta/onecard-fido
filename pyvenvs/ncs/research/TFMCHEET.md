# TF-M関連各種手順書

Trusted Firmware（TF-M）についての各種手順を掲載します。

## デバッグ関連

#### パニック時のリブートを回避
TF-Mは、デフォルトでパニック時リブートをさせるロジックを採用しています。<br>
リリース時は、もちろんこのロジックである必要がありますが、デバッグ時はパニックリブートがループしてしまい、結果として修正プログラムを書き込むことができない事態が発生します。

こちらを停止させるために、以下のプログラムを修正します。
```
${HOME}/GitHub/onecard-fido/pyvenvs/ncs/modules/tee/tfm/trusted-firmware-m/secure_fw/spm/ffm/psa_client_service_apis.c
${HOME}/GitHub/onecard-fido/pyvenvs/ncs/modules/tee/tfm/trusted-firmware-m/secure_fw/spm/ffm/utilities.c
```

該当箇所をコメントアウトすることで、パニック時のリブートを回避することができます。
```
void tfm_core_panic(void)
{
    ：
    //tfm_hal_system_reset();  <-- リブートさせている部分をコメントアウトします。
}
```


#### ブートローダーのログレベルを変更
TF-Mのブートローダーのログレベルは、デフォルトでINFOになっています。<br>
こちらをDEBUGに変更することにより、詳細ログを出力させることができます。

変更時は以下の定義ファイルを修正します。
```
${HOME}/GitHub/onecard-fido/pyvenvs/ncs/modules/tee/tfm/trusted-firmware-m/config/config_default.cmake
```

該当箇所にデバッグレベルを指定します。

```
：
set(MCUBOOT_LOG_LEVEL                   "DEBUG"     CACHE STRING    "Level of logging to use for MCUboot [OFF, ERROR, WARNING, INFO, DEBUG]")
：
```

#### TF-M設定の上書き
前述のTF-M設定`config/config_default.cmake`は、ターゲットボードごとに、設定が上書きできるようです。<br>
例えば下記のような設定`SECURE_UART1=OFF`があります。
```
set(SECURE_UART1                        OFF         CACHE BOOL      "Enable secure UART1")
```

これは実機では`SECURE_UART1=ON`となっており、デフォルトから設定上書きがされています。<br>
該当箇所は下記の設定ファイルになります。

```
${HOME}/GitHub/onecard-fido/pyvenvs/ncs/modules/tee/tfm/trusted-firmware-m/platform/ext/target/nordic_nrf/common/nrf5340/config.cmake
```

設定ファイル`nrf5340/config.cmake`を参照すると、確かに前述の上書き設定が行われていることが確認できます。
```
set(SECURE_UART1                        ON         CACHE BOOL      "Enable secure UART1")
```
