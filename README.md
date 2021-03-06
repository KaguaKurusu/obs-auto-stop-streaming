# obs-auto-stop-streaming
**配信を自動停止するOpen Broadcast Software Studio用のプラグイン**  
[![License GPL v2](https://img.shields.io/badge/license-GPLv2-blue)](LICENSE)

## 概要
設定したシーンに遷移後、設定時間経過したら自動的にOBSの配信停止をするプラグインです。

## ダウンロード
[リリースページ](https://github.com/KaguaKurusu/obs-auto-stop-streaming/releases/)よりご利用の環境に合ったファイルをダウンロードしてください。
- 32bit版をご利用の場合はrelease_x86.zip
- 64bit版をご利用の場合はrelease_x64.zip

## インストール
ダウンロードしたファイルをOBSインストールフォルダ内に解凍してください。  
(OBSをインストール時にインストール先を変更していなければ「C:\Program Files\obs-studio」フォルダ内に解凍)

## 使用方法
「配信停止するまでの時間」および「配信停止するシーン」を設定して配信を開始し、「配信停止するシーン」に遷移する。  
別のシーンに移動した場合は「配信停止するまでの時間」はリセットされ、再度「配信停止するシーン」に遷移しない限り自動終了されません。  
表示されていない場合、「メニューバー > 表示 > ドック > 自動配信停止」から再表示できます。  
※ あくまでOBSの配信停止を行うだけですので、利用プラットフォーム(YouTube等)の配信停止は別途行ってください。

## 動作環境
Windows10  
OBS 25.0.8にてテスト済み

## 既知の問題
- ディスプレイの表示スケールが100%ではない環境でドック表示した際にタイトル部分が正常に表示されない。
- ディスプレイの表示スケールが異なるディスプレイ間を移動した際に表示がおかしくなる。
- 前回と異なる位置に表示される場合がある。
- OBS起動後ドックに表示されない場合がある。
- プロファイル、シーンコレクションが複数ある場合に設定が保存されない。(現状OBS終了時に選択されているシーンコレクションの設定のみ保存する仕様です。)

## ビルド方法
### 事前準備
Microsoft Visual Studio 2019 VC++をインストールしてcmakeのあるフォルダにパスを通しておく。  
コマンドプロンプトにて以下を実行。  
`prebuild.bat`

### ビルド
コマンドプロンプトにて以下を実行。
1. x86及びx64の両方をビルドする場合  
`build.bat`
2. x86のみビルドする場合  
`build.bat x86`
3. x64のみビルドする場合  
`build.bat x64`

## ライセンス
本プラグインは[GNU GPL v2.0](LICENSE)にて公開しております。  
Copyright 2020, [来栖華紅鴉](https://twitter.com/kagua_kurusu)