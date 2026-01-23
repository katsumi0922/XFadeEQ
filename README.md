# XFadeEQ

## 📝 概要
XFadeEQ は、JUCE フレームワークを使用して開発されているオーディオプラグインです。
3つの異なる EQ 設定をスムーズにクロスフェード（XFade）させる機能の実現を目指しています。
開発を目的としたものではなく、C++ および JUCE フレームワークを用いたオーディオ信号処理の学習を主眼に置いています。

## ✨ 機能（予定含む）

### UIと信号処理

xxx

### 使い方

xxx

## 💡 ユースケース ～どう便利か～

本プラグインは、**EQカーブのオートメーションを描かずにシームレスにフィルタ特性を切り替えられる**のが最大の特長です。従来は煩雑だった複数パラメータのオートメーションを、単一のフェーダー操作に集約できます。以下3つのケースを例にとって、ミキシング・アレンジにおける有用性を説明します。

| ケース | 課題と従来アプローチ | XFadeEQ による解決 |
| :--- | :--- | :--- |
| **A. テイク間補正** <small><br>＜ミキシング＞<br>録音テイク間の音質の差を動的にマッチさせる</small> | **課題**: ボーカルや生楽器のミキシングにおいて、録音状況が異なるテイクを混ぜると**音質に差が出てしまう（テイク間の音色ズレ）**。<br>**従来**: 音質をマッチさせるため、多数のEQパラメータ（周波数・ゲイン・Q値）に対して綿密なオートメーションを描く。 | 各テイク用の補正をEQ_A/B/Cに設定。クロスフェーダー1本のオートメーションだけで、テイクの切り替わりに応じたシームレスな特性遷移を実現。 |
| **B. 動的なピーク整地** <small><br>＜ミキシング＞<br>周波数ごとのダイナミクスを動的に整地する</small> | **課題**: 特にボーカルや生楽器は**周波数ごとのダイナミクスが大きく**、特定の瞬間だけ周波数ピークを抑える必要がある。<br>**従来**: ダイナミックEQやディエッサーを使う or EQバンドのゲインを直接オートメーションする（後者は操作量が膨大になりがち） | 「フラット特性（EQ_A）」と「ピーク除去（EQ_B/C）」を用意。フェーダーを動かすだけで、複数バンドを同時に、かつ音楽的・ダイナミックに一括制御できる。 |
| **C. 音楽的な音色変化** <small><br>＜アレンジ＞<br>音楽的にEQを用いて音色を変える</small> | **課題**: ラジオボイス（バンドパス）加工等、音色を動的に変える際にバイパスON/OFFではなく丁寧に遷移させたい。<br>**従来**: ローパスとハイパス両方のカットオフ周波数を連動させて描くなど、オートメーション作成に複雑な手間が発生。 | 「ドライ音（EQ_A）」から「加工後の音（EQ_B/C）」へクロスフェード。フェーダー1本の直感的な操作で、あらゆるフィルタを伴う滑らかな質感の変化を表現可能。 |

### XFadeEQのメリット

上記の通り、従来は多くのEQパラメータに対して個別に描いていたオートメーションが、**クロスフェーダー1本**に集約されます。

1. **オートメーションの簡素化**: クロスフェーダー1本でシームレスにEQ特性を動的調整できる
2. **後からの調整が容易**: EQカーブ自体はオートメーションしないため、後からパラメータを変更してもオートメーションが破綻しない
3. **直感的なワークフロー**: 「この音色」と「あの音色」を行き来するという音楽的発想を、そのまま操作に反映

## 🛠️ 開発環境
- **Framework:** JUCE (w/ Projucer, w/o CMake)
- **Language:** C++20
- **IDE:** Visual Studio 2026
- **Target Formats:** VST3 (AU, Standalone)
- **JUCEの置き場所:** C:\JUCE

## 📦 セットアップ / ビルド方法
1. JUCE をインストール
2. `XFadeEQ.jucer` を Projucer で開き、Visual Studio のプロジェクトを書き出し（Save Project and Open in IDE）
3. IDE でビルドを実行

## 📚 学習の軌跡 (Dev Logs)
ここでは、開発中に学んだことや直面した課題を記録していきます。

### 環境立ち上げ ～ 初期チュートリアル
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
    - 「押され続けている間」の音出しは、`processBlock` 内の オーディロループ が担当。
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
    - (今回はそこまで手が回らなそうだけど）APVTSを使うと **state保存/復元（DAWプロジェクト保存）**が簡単に実装できる

### juce_dspとか基本的な信号処理
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
- https://panda-clip.com/juce-introduction-to-dsp3/
  - juce::dsp::ProcessorChainを使えば複数のdsp処理をまとめて扱える

### その他
- https://note.com/leftbank/n/nc60ebc8bf3d2
- https://www.youtube.com/watch?v=i_Iq4_Kd7Rc
- https://panda-clip.com/juce-modify-simple-highpass1/

### AI(LLM)の使用について
- 本開発はJUCEフレームワークの学習を主眼と置いておりコーディングにはAIを使用していない
- ただし以下のようなケースでは積極的にAIを使用している
  - ソースコードへのコメント追記
  - ソースコード整形（変数名なども）
  - README等ドキュメントの添削/整形
  - リファクタリングのアドバイスを貰う
