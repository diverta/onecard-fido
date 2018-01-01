# 調査情報

## 各種調査・検証

* <b>[U2Fローカルテストサーバー](u2f-test-server/README.md)</b><br>
ChromeでBLE U2F対応調査／開発を行う際に使用する、ローカルテスト用のU2Fサーバーを構築しました。<br>
U2Fローカルテストサーバーについて、概要説明や、利用手順を掲載しております。

* <b>[Chrome OSでのU2F認証テスト](CHROMEOSTEST.md)</b><br>
テスト用のChromeエクステンションを使用し、[U2F認証テストサイト](https://crxjs-dot-u2fdemo.appspot.com/)でU2F認証ができることを確認しました。<br>
その時の手順および結果を掲載しております。

* <b>[Chrome(macOS版)でのBLE U2F対応調査](u2f-ble-helper-macOS/README.md)</b><br>
macOSでサポートされている「Web Bluetooth API」を使用して、BLE U2Fクライアントが実現できるかどうか調査しました<br>
結果としては、実現不可能という判断となりました。<br>
その時の検討内容および動作確認結果などを掲載しております。

* <b>[\[WIP\] ブラウザーエクステンションの調査](CHROMEBLEEXT.md)</b><br>
PC環境で動作するFIDO U2F BLEエクステンションの調査です。

## 補足説明

（未作成）

後日、随時追加していく予定です。

## TODO

* <b>Android環境でのサポート状況確認</b><br>
Android向けGoogle Playで既にサポートずみ（とのこと）である、BLE U2Fサービスが利用できるかどうかを、実機で確認する予定です。<br><br>
[ご参考] Googleサポート状況に関する議論：<br>
https://groups.google.com/a/fidoalliance.org/forum/#!topic/fido-dev/-hT1UF0FKTo
