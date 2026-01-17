# XFadeEQ

## 概要
XFadeEQ は、JUCE フレームワークを使用して開発されているオーディオプラグインです。
2つの異なる EQ 設定をスムーズにクロスフェード（XFade）させる機能の実現を目指しています。
開発を目的としたものではなく、C++ および JUCE フレームワークを用いたオーディオ信号処理の学習を主眼に置いています。

## 機能（予定含む）
xxx

## 使い方
xxx

## ユースケース
xxx

## 開発環境
- **Framework:** JUCE (Projucer)
- **Language:** C++20
- **IDE:** Visual Studio 2026
- **Target Formats:** VST3, AU, Standalone
- **JUCEの置き場所:** C:\JUCE

## セットアップ / ビルド方法
1. JUCE をインストール
2. `XFadeEQ.jucer` を Projucer で開き、Visual Studio のプロジェクトを書き出し（Save Project and Open in IDE）
3. IDE でビルドを実行

## 学習の軌跡 (Dev Logs)
ここでは、開発中に学んだことや直面した課題を記録していきます。

### 立ち上げ ～ 初期チュートリアル
- https://qiita.com/COx2/items/fa1aceb5013f92457469
  - 主にこれを見た
- https://trap.jp/post/2307/
  - たまにこっちも見た（`AudioPluginHost` の使い方とか）

### 基本的な実装フロー
- https://qiita.com/aike@github/items/67286a9297a7eccfeb39
  - クラスの構造とか全容
  - `processBlock()` でバッファにデータ埋めていく感じとかざっくり
  - MIDIノートはとりあえず終わりと始まりだけ考慮している
    - `isNoteOn` と `isNoteOff` は MIDI メッセージ（イベント）が届いた **瞬間** にのみ実行される。鍵盤が「押された瞬間」と「離された瞬間」にのみ処理を行う。
    - 「押され続けている間」の音出しは、`processBlock` 内の オーディオループ が担当。
- https://qiita.com/aike@github/items/1105e74e01d006a3880e
  - エフェクトの場合どうするかとか
  - スライダーの扱い方。インターフェースを使う仕組みを学習。
- https://qiita.com/aike@github/items/544a7af2ec7188e85593
  - **Atomic**: 気軽に使える。「単なる数値」が必要なら `Atomic` を、「DAWから操作可能なパラメータ」が必要なら `AudioParameterFloat` を使用。
  - **鍵盤UI**: `MidiKeyboardState` ＋ `MidiKeyboardComponent` を Editor に持たせて表示。
  - `keyboardComponent(keyboardState, ...)`: 初期化子リストで構築が必要なコンポーネント。

### パラメーター関連
- https://qiita.com/COx2/items/a0dc18ef29685f951257
    - 汎用的なGUIを生成してくれるGenericAudioProcessorEditorクラス
    - 信号処理に専念したい序盤をこれを使うと良さそう
- https://qiita.com/perfectpanda145/items/45f637fe54a3a76e60c9
    - APVTS: 音声処理クラス(XXAudioProcessor)とエディタクラス（XXAudioProcessorEditorクラス）の両方でアクセスできるパラメータが容易に定義
    - GenericAudioProcessorEditorは、Processorが持ってる“AudioProcessorParameter群”を自動でUI化
    - APVTSは、その“AudioProcessorParameter群”を まとめて定義・管理しやすい
    - APVTSを使うと **state保存/復元（DAWプロジェクト保存）**が簡単に

### juce_dspとか基本的な信号処理(これ理解すればとりあえずグライコは作れそう)
- https://trap.jp/post/1558/
  - juce_dspの使い方
  - 本当だったら、Gain部分はサンプルごとにn倍したり、Pan部分はLRチャンネルに重みづけをしたりするのですが、このレベルの簡単な信号処理だったらJUCEのdsp::Gainとdsp::Pannerモジュールを使うことが出来ます。JUCEはかなりの数の信号処理機をデフォルトで持っているので、余程のこだわりが無ければこれらのモジュールを使ってしまうのが便利です。
  - juce::dsp::AudioBlockでラッパーするとbufferがjuce_dspで扱いやすくなる
  - `juce::dsp::ProcessContextReplacing<float> context(audioBlock)`と言った感じで前処理し、`process(context)`すると信号処理が適用できる
- https://qiita.com/Aogiri-m2d/items/ac012a3b9cb3e50e1b07
  - juce_dspに入っているフィルタの使い方とか
  - 情報量そこそこあるので読み飛ばしながら
  - AudioBlockとProcessContextReplacingは何者なのか
  - 入力～フィルタまでの流れ
    1. フィルタ用processor（例：`IIR::Filter`）をメンバに用意し、ステレオ等にしたい場合は `ProcessorDuplicator<Filter, Coefficients(State)>` でラップして「係数（状態）を共有できる形」にする。
    2. `prepareToPlay()` で `ProcessSpec`（sampleRate/ch数/blockサイズ）を埋めて、各processorに `prepare(spec)` を呼んで初期化する。
    3. `processBlock()` で係数/パラメータを更新し、`AudioSampleBuffer`→`AudioBlock`→`ProcessContextReplacing` と包んで、`lsf.process(context)` のようにチェーン順に `process()` を呼べば入力信号にフィルタがかかる。

### その他
- https://note.com/leftbank/n/nc60ebc8bf3d2
- https://www.youtube.com/watch?v=i_Iq4_Kd7Rc
- https://panda-clip.com/juce-modify-simple-highpass1/

### AI(LLM)の使用について
- 本開発はJUCEフレームワークの学習を主眼と置いておりコーディングにはAIを使用していない
- ただし以下のようなケースでは積極的にAIを使用している
  - コメント追記
  - ソースコード整形（変数名なども）
  - README等ドキュメントの添削/整形
