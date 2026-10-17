# RecMuter

Pro Tools 用の **AAX Native** プラグインです。ホストが **再生中または録音中のときだけ** 音声をミュートし、停止中はそのまま音を通します。

例えば、停止中だけ聴きたいトークバック／モニター用のトラックや、再生・録音中に鳴ってほしくない信号の制御に使えます。

## 機能

| パラメータ | 内容 | 既定値 |
| --- | --- | --- |
| Mute When | ミュートする条件。`Play or Record`（再生または録音）/ `Record Only`（録音中のみ）/ `Play Only`（再生中のみ・録音中は通す） | `Play or Record` |
| Fade | ミュート切り替え時のフェード時間（0〜200 ms）。クリックノイズを防ぐ | 10 ms |

- 画面上部に現在の状態（`MUTED` / `PASS`）とトランスポート状態（Stopped / Playing / Recording）を表示
- モノラル・ステレオ・サラウンド（入出力が同じ構成）に対応
- レイテンシー 0

### 動作上の注意

- 判定には Pro Tools から渡されるトランスポート情報（`IsTransportPlaying` と録音状態）を使います。
- Pro Tools は停止中、入力モニターしていないオーディオトラックの Insert を処理しないことがあります。停止中に音を通したい信号は **Aux Input トラック** や、入力モニター状態のトラックにインサートしてください。

## 構成

```
CMakeLists.txt            ビルド設定（JUCE を FetchContent で自動取得）
Source/MuteLogic.h        ミュート判定とフェード処理（JUCE 非依存）
Source/PluginProcessor.*  オーディオ処理・パラメータ
Source/PluginEditor.*     プラグイン画面
Tests/MuteLogicTests.cpp  MuteLogic の単体テスト
```

フレームワークには [JUCE](https://juce.com/) 8 を使用しています。JUCE 8 には AAX SDK が同梱されているため、AAX SDK を別途ダウンロードする必要はありません。

## ビルド方法

必要なもの: CMake 3.22 以上、Git、および以下のいずれか

- macOS: Xcode（AAX / AU / VST3 をビルド）
- Windows: Visual Studio 2022（x64。AAX / VST3 をビルド）

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # macOS は -G Xcode を付けても可
cmake --build build --config Release
ctest --test-dir build -C Release   # 単体テスト
```

成果物は `build/RecMuter_artefacts/Release/AAX/RecMuter.aaxplugin` に出力されます。

> Linux では AAX をビルドできません（VST3 と単体テストのみビルドされます）。

## Pro Tools への導入

1. `RecMuter.aaxplugin` を以下にコピーします。
   - macOS: `/Library/Application Support/Avid/Audio/Plug-Ins/`
   - Windows: `C:\Program Files\Common Files\Avid\Audio\Plug-Ins\`
2. Pro Tools を起動し、トラックの Insert から **Dynamics > RecMuter** を選択します。

### PACE 署名について（重要）

製品版の Pro Tools は **PACE（iLok）で署名された AAX プラグインしか読み込みません**。

- **開発・動作確認**: Avid の AAX 開発者プログラムに登録すると入手できる **Pro Tools Developer** なら、未署名のプラグインを読み込めます。
- **製品版 Pro Tools で使う**: Avid 開発者アカウントと PACE Eden（`wraptool`）を取得し、ビルド後に署名してください。

```bash
# 署名の例（macOS）。アカウント情報や WCGUID は PACE から発行されたものを使用
wraptool sign --verbose --account <PACE アカウント> --wcguid <WCGUID> \
  --signid "<Developer ID Application 証明書名>" \
  --in  build/RecMuter_artefacts/Release/AAX/RecMuter.aaxplugin \
  --out build/RecMuter_artefacts/Release/AAX/RecMuter.aaxplugin
```

## CI

`.github/workflows/plugin-build.yml` で macOS / Windows / Linux のビルドと単体テストを実行し、macOS・Windows の AAX / VST3（/ AU）を Artifacts としてアップロードします（未署名）。

## AI アシスタント運用方針

このリポジトリは **Claude をメインの AI アシスタント** として利用し、**Cursor / GitHub Copilot をサブツール** として併用するワークフローを想定しています。開発ルールの **正本は [`AGENTS.md`](./AGENTS.md)** で、各ツール向けの指示書はそれを参照する構成です。

| ツール | ファイル | 役割 |
| --- | --- | --- |
| 汎用 AI エージェント | [`AGENTS.md`](./AGENTS.md) | **正本**。すべてのルールはここに集約 |
| Claude Code | [`CLAUDE.md`](./CLAUDE.md) | `AGENTS.md` を参照 |
| Cursor | [`.cursor/rules/project.mdc`](./.cursor/rules/project.mdc) | `AGENTS.md` を参照 |
| GitHub Copilot | [`.github/copilot-instructions.md`](./.github/copilot-instructions.md) | `AGENTS.md` を参照 |

## ライセンス

MIT（JUCE および AAX SDK はそれぞれのライセンスに従います）
