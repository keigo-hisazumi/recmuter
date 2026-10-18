# RecMuter

Pro Tools 用の **AAX Native** プラグインです。ホストが **再生中または録音中のときだけ** 音声をミュートし、停止中はそのまま音を通します。停止中だけ聴きたいトークバック／モニター用のトラックや、再生・録音中に鳴ってほしくない信号の制御に使えます。

## 主な機能

- 再生中・録音中だけ音声をミュートし、停止中は音を通す
- ミュートする条件を `Play or Record` / `Record Only` / `Play Only` から選べる
- ミュート切り替え時にフェードをかけてクリックノイズを防ぐ（0〜200 ms）
- 画面上部に現在の状態（`MUTED` / `PASS`）とトランスポート状態（Stopped / Playing / Recording）を表示
- モノラル・ステレオ・サラウンド（入出力が同じ構成）に対応
- レイテンシー 0

## パラメータ

| パラメータ | 内容 | 既定値 |
| --- | --- | --- |
| Mute When | ミュートする条件。`Play or Record`（再生または録音）/ `Record Only`（録音中のみ）/ `Play Only`（再生中のみ・録音中は通す） | `Play or Record` |
| Fade | ミュート切り替え時のフェード時間（0〜200 ms）。クリックノイズを防ぐ | 10 ms |

## 動作上の注意

- 判定には Pro Tools から渡されるトランスポート情報（`IsTransportPlaying` と録音状態）を使います。
- Pro Tools は停止中、入力モニターしていないオーディオトラックの Insert を処理しないことがあります。停止中に音を通したい信号は **Aux Input トラック** や、入力モニター状態のトラックにインサートしてください。

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

## 技術スタック

- C++17
- JUCE（CMake の FetchContent で自動取得。JUCE に AAX SDK が同梱されているため、AAX SDK を別途ダウンロードする必要はありません）
- CMake / CTest（ビルドと単体テスト）
- プラグイン形式: AAX（macOS / Windows）、VST3、AU（macOS）
- GitHub Actions（macOS / Windows / Linux でのビルドと単体テスト）

## 開発

### 必要な環境

- CMake 3.22 以上
- Git
- macOS: Xcode（AAX / AU / VST3 をビルド。Intel / Apple Silicon のユニバーサルバイナリ、macOS 11.0 以降が対象）
- Windows: Visual Studio 2022（x64。AAX / VST3 をビルド）

Linux では AAX をビルドできません（VST3 と単体テストのみビルドされます）。

### コマンド

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # 設定（macOS は -G Xcode を付けても可）
cmake --build build --config Release             # ビルド
ctest --test-dir build -C Release                # 単体テスト
```

成果物は `build/RecMuter_artefacts/Release/AAX/RecMuter.aaxplugin` に出力されます（VST3 / AU は同じ `Release/` 配下の `VST3/` / `AU/`）。

### CMake のオプション

| オプション | 内容 | 既定値 |
| --- | --- | --- |
| `RECMUTER_JUCE_DIR` | ローカルの JUCE ディレクトリ。空なら FetchContent で自動取得 | 空 |
| `RECMUTER_BUILD_TESTS` | ミュート判定ロジックの単体テストをビルドする | `ON` |

## ディレクトリ構成

```text
CMakeLists.txt            ビルド設定（JUCE を FetchContent で自動取得）
Source/
  MuteLogic.h             ミュート判定とフェード処理（JUCE 非依存）
  PluginProcessor.*       オーディオ処理・パラメータ
  PluginEditor.*          プラグイン画面
Tests/
  MuteLogicTests.cpp      MuteLogic の単体テスト
```

## デプロイ

Web への公開や自動リリースはありません。配布は次の方法で行います。

- PR と `main` への push ごとに `.github/workflows/plugin-build.yml` が macOS / Windows / Linux でビルドと単体テストを実行し、macOS・Windows の AAX / VST3（/ AU）を Artifacts（`RecMuter-macOS` / `RecMuter-Windows`）としてアップロードします（未署名）
- `main` への push ごとに `.github/workflows/release-drafter.yml` が GitHub Releases のリリースノートの下書きを更新します（ビルド成果物は添付されません）
- 製品版 Pro Tools で使う場合は、上記の成果物またはローカルのビルド結果を PACE で署名してから [Pro Tools への導入](#pro-tools-への導入) の手順でインストールします

## 開発ルール

開発ルールは [AGENTS.md](./AGENTS.md) を参照してください。
貢献の手順は [CONTRIBUTING.md](./CONTRIBUTING.md) を参照してください。

## ライセンス

MIT（JUCE および AAX SDK はそれぞれのライセンスに従います）
