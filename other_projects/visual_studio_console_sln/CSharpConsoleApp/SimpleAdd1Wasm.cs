using System;

namespace CShort.Samples
{
    public class WasmBytesSample
    {
        /// <summary>
        /// 「渡された数値に1を足す」wasmバイナリのシンプルなバイト列を返す関数
        /// </summary>
        /// <returns>WASMバイナリ（バイト配列）</returns>
        public static byte[] GetSimpleAdd1WasmBytes()
        {
            // 簡単なWASMバイナリ（渡されたi32に1を加えて返す関数 "add1" のみ実装）
            // 各セクションの意味はコメント参照
            return new byte[] {
                // --- ヘッダー ---
                0x00, 0x61, 0x73, 0x6D, // "\0asm" マジックナンバー
                0x01, 0x00, 0x00, 0x00, // バージョン

                // --- Typeセクション (関数の型宣言) ---
                0x01,       // TypeセクションのID
                0x07,       // セクションサイズ（7バイト）
                0x01,       // 型エントリは1つ
                0x60,       // func型
                0x01, 0x7F, // 引数は1つ(i32)
                0x01, 0x7F, // 戻り値は1つ(i32)

                // --- Functionセクション (関数本体と型の紐づけ) ---
                0x03,       // FunctionセクションID
                0x02,       // セクションサイズ（2バイト）
                0x01,       // 関数数は1つ
                0x00,       // タイプインデックス（typeセクションの0番目）

                // --- Exportセクション ("add1"関数をエクスポート) ---
                0x07,           // ExportセクションID
                0x07,           // セクションサイズ（7バイト）
                0x01,           // エクスポートは1つ
                0x04,           // 名前の長さ[4]
                0x61, 0x64, 0x64, 0x31, // "add1"
                0x00,           // エクスポート種別：関数
                0x00,           // 関数インデックス0番

                // --- Codeセクション (関数の命令列) ---
                0x0A,       // CodeセクションID
                0x09,       // セクションサイズ（9バイト）
                0x01,       // 関数数1つ
                0x07,       // 関数本体サイズ7バイト
                0x00,       // ローカル変数数0
                0x20, 0x00, // local.get 0
                0x41, 0x01, // i32.const 1
                0x6A,       // i32.add
                0x0B        // end
            };
        }
    }
}