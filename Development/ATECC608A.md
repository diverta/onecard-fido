# セキュアエレメント「ATECC608A」について

## 概要

超小型外付けセキュアIC「[ATECC608A](https://www.mouser.jp/ProductDetail/Microchip-Technology/ATECC608A-MAHDA-T?qs=sGAEpiMZZMve4%2FbfQkoj%252BNx3hPbDs5d66otQ2I4K6nk%3D)」の具体的な使用方法について調査を行います。<br>
大きさは 3mm x 2mm です。

### 目的と背景
Yubico社から発売されているWebAuthn対応のトークン「[Security Key by Yubico](https://yubikey.yubion.com/yubikey_lineup.html)」に実装されている「`resident key`オプション」という機能を、[MDBT50Q Dongle](https://github.com/diverta/onecard-fido/tree/master/FIDO2Device/MDBT50Q_Dongle)に実装するためには、Flash ROM以外に複数の秘密鍵を保管できるデバイスが追加で必要となります。

また、[nRF52840版FIDO2認証器](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.3.0)のmbed OS移植先マイコンとして選定した「STM32F411RE」は、秘密鍵やハッシュ等の演算機能を有しないため、移植が困難となっています。

以上の２つの理由から、掲題の「ATECC608A」を基板上に増設し、各々のタスクを再開できないかどうか、調査しようというものです。

## 秘密鍵

「ATECC608A」では、FIDO関連処理で使用するEC秘密鍵を、プログラムではなくハードウェアにより計算して生成し、ICの内部メモリー（バンク）に永続化します。[注1]

「ATECC608A」内部で生成された秘密鍵は、参照することが不可能です。<br>
これにより、秘密鍵漏洩によるなりすましを抑止することができます。

一方、FIDOにおけるユーザー登録処理では、外部機関で証明書を生成する必要があります。<br>
この際、秘密鍵から生成される公開鍵を「ATECC608A」から読み出し、証明書要求を生成する必要があります。

[注1] ATECC608Aに秘密鍵を作成させるためのハードウェア操作（I2Cコマンド投入）を、ユーザーアプリケーションから行うことができるようにするため、製造元（Microchip社）から、アプリケーションAPIが用意されています。
