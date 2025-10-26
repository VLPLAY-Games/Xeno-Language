[![Version](https://img.shields.io/badge/Version-0.1.0-lightgrey.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-ESP32-orange.svg)](#)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-brightgreen.svg)](#)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

- [Read in Russian](README.ru.md) 
- [Read in English](README.md)

# Xeno Language

**Xeno Language** — ESP32（Arduino）エコシステム向けのコンパクトで安全なインタプリタ言語と仮想マシン。  
実装形態：**コンパイラ → バイトコード → VM**。数値、文字列、分岐、ループ、基本的なGPIO制御の組み込みコマンドを備えています。

**開発者：VL_PLAY Games**

---

## 🚀 クイックピッチ

✨ **ハイライト**
- `.xeno` ファイルまたは埋め込みソース文字列からコードを実行できます。  
- 軽量でほとんどのESP32プロジェクトへ簡単に組み込めます。  
- ホビイスト、教育、小規模オートメーション向けに設計されています。

⚡️ **パフォーマンス**
- ベンチマークでは、Xenoは同等のネイティブC++に対して概ね **約22倍遅い** と報告されています（ワークロードに依存）。詳細は `benchmark.ino` を参照してください。  
- MicroPythonやLuaなどの他のインタプリタ言語と比べても同程度のオーダーです — 正確な差はコードや使い方によって変わります。ワークロード固有の比較は `benchmark.ino` を使用してください。
### ⚔️ 言語パフォーマンス比較

| 機能 / 言語                  | **Xeno** 🧠       | **MicroPython** 🐍       | **Lua (NodeMCU)** 🌙    | **C++ (ネイティブ)** ⚙️     |
|-----------------------------|:-----------------:|:------------------------:|:----------------------:|:---------------------------:|
| 実行速度（C++比）           | ~22×遅い          | ~18×遅い *(概算)*        | ~20×遅い *(概算)*       | 🥇 基準                     |
| メモリ使用（RAM）           | 低（約20 KB）     | 中（約30 KB）            | 中（約25 KB）           | 高（手動管理）              |
| ファイル実行                | ✅ `.xeno` ファイル | ✅ `.py` ファイル        | ✅ `.lua` ファイル      | ⚠️ コンパイルのみ            |
| ハードウェアアクセス (GPIO等)| ⚠️ 基本（LEDのみ、開発中） | ✅ 豊富                 | ✅ 豊富                 | ✅ 完全                      |
| 組み込みやすさ              | ⭐️⭐️⭐️⭐️⭐️        | ⭐️⭐️⭐️⭐️               | ⭐️⭐️⭐️⭐️               | ⭐️⭐️                       |
| 言語の簡単さ                | ほどほど簡単       | 簡単                     | 中程度                  | 複雑                        |
| C++プロジェクトへの埋め込み | ✅ 完全に埋め込み可 | ⚠️ 制限あり（別ランタイム） | ⚠️ 制限あり（別ランタイム） | —                          |
| 理想的なユースケース         | 教育、簡単なロジック、アプリ内スクリプト | プロトタイピング、IoT | スクリプト自動化         | パフォーマンス重視          |

> ⚡ **注記:**  
> - **Xeno** の速度比は **ESP32-C3 @160 MHz** で計測されたものです。MicroPythonやLuaの値は概算で、ワークロードにより変動します。詳細は `benchmark.ino` を参照してください。  
> - **Xenoの利点:** 多くのESP32ワークフローでのMicroPythonやLuaと異なり、**Xenoは既存のC++ファームウェアに直接組み込めます** — 別のインタプリタ用ファームウェアは不要です。  
> - **GPIO注記:** 現在XenoのハードウェアアクセスはLED制御に限定されています。将来のアップデートで拡張予定です。

---

## システム要件

### ハードウェア
- 任意のESP32系マイクロコントローラ（ESP32、ESP32-S2など）  
- 推奨空きフラッシュ：**≥ 60 KB**  
- 推奨空きRAM：**≥ 20 KB**

### ソフトウェア
- ESP32 Arduino core: **3.2.0+**  
- Arduino IDE: **2.0+**

> 補足: 実際のメモリ使用量は有効化した機能、文字列、含めた例に依存します。プロジェクトでメモリが厳しい場合は、未使用機能を削り、設定ヘッダの `MAX_*` 値を減らしてください。

---

## クイックスタート

### インストール
`xenoLang` フォルダをArduinoプロジェクトにコピーしてください。リポジトリには `examples/` に `.ino` スケッチの例が含まれます。

### 最小限の Arduino / ESP32 例
```cpp
#include "xenoLang/xeno.h"

Xeno xeno;
  // Increase allowed instruction count (optional)
  // Example: xeno.setMaxInstructions(200000); // raises the VM instruction limit

void setup() {
  Serial.begin(115200);
  delay(1000);

  String program = R"(
    print "Hello from Xeno!"
    set a 10
    set b 20
    print "a + b = "
    set c a + b
    print $c
    halt
  )";

  if (!xeno.compile(program)) {
    Serial.println("Compile error");
    return;
  }

  xeno.run();
}

void loop() {
  // Optionally use xeno.step() for single-step execution or poll status
}
```

---

## 言語概要

### 基本コマンド
- `print "text"` — リテラル文字列を出力。  
- `print $var` — 変数の値を出力。  
- `set var expr` — 変数に代入（式をサポート）。  
- `input var` — Serial経由で入力を要求（文字列として保存）。  
- `halt` — プログラム実行を停止。  
- `led <pin> on|off` — 許可されたGPIOピンを切り替え。  
- `delay <ms>` — ミリ秒単位で待機（上限あり）。  
- スタック & 算術: `add`, `sub`, `mul`, `div`, `mod`, `abs`, `pow`, `sqrt`, `max`, `min`.  
- 制御フロー: `if ... then ... else ... endif`, `for var = start to end ... endfor`.  
- 一行コメントは `//`。

### クイック構文スニペット
```
// Comments
print "Hello World"
led 13 on
delay 1000

// Variables
set x 10
set name "Xeno"
print $x

// Arithmetic
set result (x + 5) * 2

// Conditionals
if x > 5 then
    print "Larger than 5"
endif

// Loops
for i = 1 to 10
    print $i
endfor
```

### 演算 & 比較
- 算術: `+ - * / % ^`, `abs()`  
- 比較: `== != < > <= >=`  
- 制御: `if/then/else/endif`, `for/endfor`  
- 周辺機能: `print`, `led on/off`, `delay`

---

## 仮想マシン（VM）

- コンパイル済みバイトコードをランタイムスタック、変数テーブル、文字列テーブルを使って実行します。  
- スタックオーバーフロー、無効オペコード、ゼロ除算、境界チェックなどの組み込み安全チェックを備えています。  
- APIの主な機能（クラス `Xeno`）:
  - `bool compile(const String& source)` — ソースをバイトコードにコンパイル。  
  - `bool run()` — コンパイル済みバイトコードを実行。  
  - `void step()` — 単一VM命令を実行。  
  - `void stop()` — 実行を停止。  
  - `bool isRunning() const` — 実行中かどうかをチェック。  
  - `void printCompiledCode()` — バイトコード＋文字列テーブル／デバッグ情報を出力。  
  - `void setMaxInstructions(uint32_t max_instr)` - VM命令上限を引き上げる  
  - バージョン／情報取得子: `getLanguageVersion()`, `getCompilerVersion()`, `getVMVersion()`.

---

## バイトコード / オペコード（概要）

一般的なVMオペコード例：
```
OP_NOP, OP_PRINT, OP_PRINT_NUM, OP_PUSH, OP_POP,
OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
OP_POW, OP_ABS, OP_SQRT, OP_MAX, OP_MIN,
OP_STORE, OP_LOAD,
OP_JUMP, OP_JUMP_IF,
OP_INPUT, OP_DELAY, OP_LED_ON, OP_LED_OFF,
OP_PUSH_FLOAT, OP_PUSH_STRING,
OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,
OP_HALT
```

---

## セキュリティ & 制限

ESP32を保護し実行時障害を避けるため、Xenoは保守的な制限を課しています:
- `MAX_STRING_LENGTH`（例: 256）  
- `MAX_VARIABLE_NAME_LENGTH`（例: 32）  
- `MAX_EXPRESSION_DEPTH`（例: 32）  
- `MAX_LOOP_DEPTH`, `MAX_IF_DEPTH`（設定可能）  
- `MAX_STACK_SIZE`（例: 256）  
- 許可されるGPIOピンは `xeno_security.h` 内で制限されており、許可されていないピンの制御はブロックされます。  
- バイトコードの検証はコンパイル／ロード時に行われます。

---

## 例 & ベンチマーク
リポジトリの `examples/` を参照:
- `comparison.ino` — if/else と比較の例。  
- `float_string.ino` — 浮動小数点と文字列処理。  
- `for_loop.ino` — ループと点滅の例。  
- `input_max_min.ino` — 入力処理 + 数学関数。  
- `math.ino`, `math2.ino` — 数学テストとベンチ。  
- `benchmark.ino` — ネイティブC++とXeno VMの性能比較。

---

## デバッグのヒント
- ログと入力プロンプトのために常に Serial（例: 115200）を開いてください。  
- `CRITICAL ERROR: Stack overflow` が出たら、式の複雑さを減らすか慎重に `MAX_STACK_SIZE` を増やしてください。  
- GPIOコマンドが失敗する場合は、`xeno_security.h` の許可ピンリストを確認してください。

---

## ロードマップ & 今後の予定
🎯 今後の予定:
- コア機能の継続的な開発。  
- 最適化の改善とVMオーバーヘッドの削減。  
- **XenoOS** との統合強化と `.xeno` 編集用の良いツール群の提供。  
- 追加の例、CIパイプライン、PlatformIOテンプレートの追加。

---

## コントリビュート
貢献、問題報告、プルリクエストは歓迎します。貢献する際は:
- Apache 2.0 ライセンスを尊重し、明確なコミットメッセージを付けてください。  
- テスト／例を含む小さく焦点の定まったPRを送ってください。

---

## ライセンス
このプロジェクトは **Apache License 2.0** の下でライセンスされています。詳細は `LICENSE` ファイルを参照してください。

**Xeno Language** — 開発: **VL_PLAY Games**.
