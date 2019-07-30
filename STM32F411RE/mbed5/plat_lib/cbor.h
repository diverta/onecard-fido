//
// CBORライブラリーについて
//
// このmbedアプリケーションでは、Intelの"tinycbor.a"は使用せず、
// ARMのライブラリー「ARMmbed/tinycbor」を、内容を変えずに、
// そのまま tinycbor フォルダーに導入して使用します。
//   URL = https://github.com/ARMmbed/tinycbor.git
// 
// 他方、業務処理コードでは”cbor.h”をインクルードしていますが、
// これは「ARMmbed/tinycbor」の"tinycbor.h"と同一内容なので、
// このファイルから、"tinycbor.h"にインクルードさせる構成としています。
// 
#include "tinycbor.h"
