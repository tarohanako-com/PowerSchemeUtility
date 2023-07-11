# PowerSchemeUtility (Windows電源モード設定ツール)

## ✅　はじめに

PowerSchemeUtilityは、Windows11から変更に伴い階層が深くなった、Windowsの電源モード変更を補助するツールです👏
![image](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/37ec248d-2e40-44fa-bea9-81535408f32e)

ツール起動して、ボタンを押すだけで、電源プランが変更できます。
![Animation](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/801ac9e5-24d2-4136-9a8b-1b2feb56f581)

## ✅　くわしく

Windows 10では、タスクバーから、スライダーを動かすことで簡単に電源モードを変更できていました。
![image](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/00fab962-776e-47bb-88ca-192a2675f8ff)

しかし、Windows 11では、「設定」に統合されたために、毎回「設定」を開く必要があり、ワンタッチでは変更できなくなりました。
![image](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/04769d67-1eb9-45f8-8b00-66148f0d6b74)

そこで、Windows11でも、ワンタッチで簡単に変更できるようにツール化しました。

## ✅　ひつよう

* Visual Studio 2015-2022 再頒布可能パッケージ
  * https://aka.ms/vs/17/release/vc_redist.x86.exe
  * https://aka.ms/vs/17/release/vc_redist.x64.exe
* .NET Framework 4.8 ランタイム
  * https://dotnet.microsoft.com/ja-jp/download/dotnet-framework/thank-you/net48-web-installer 

## ✅　つかいかた

### 1. PowerSchemeUtility.exeを実行
![image](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/3c618271-58ff-4a4f-8c7f-c2774ab0ebda)

### 2. 変更したい電源プランを選択する
![image](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/9f664187-f24d-496a-899a-842932c9907d)

### 3. プランが変更される
![image](https://github.com/tarohanako-com/PowerSchemeUtility/assets/132762594/be7016de-ea1d-4f74-a98b-420ef3a64563)

## ✅　便利なつかいかた

### 常駐アプリケーションとして起動する・しない設定
  * 常駐アプリケーションとして起動することでタスクバーの通知領域に常駐します
  * タスクバーの通知領域をクリックすることで素早く呼び出すことができます
  * デフォルトの状態は、常駐アプリケーションとして起動しない設定です
  * 「PowerSchemeUtility.exe.config」内のRunningAtBackground設定値を変更してください。

### 常駐アプリケーションとして起動する際、起動時に画面を表示する・しない設定
  * 常駐アプリケーションとして起動する際に、画面を表示するかどうかを設定できます
  * スタートアップに登録する場合は、画面を表示せずに起動させることができます
  * デフォルトの状態は、起動時に画面を表示する設定です
  * 「PowerSchemeUtility.exe.config」内のShowWindow設定値を変更してください。

### モードを変更したら終了する・しない設定
  * モードを変更した際に画面を閉じたい場合に設定できます
  * 常駐アプリケーションとして起動している場合は、終了する代わりにウィンドウを非表示にします
  * デフォルトの状態は、モードを変更しても終了しない設定です
  * 「PowerSchemeUtility.exe.config」内のChangeModeAfterExit設定値を変更してください。

## ✅　そのほか
* 免責
  * このソフトウェアを使用して発生したいかなる損害にも、作者は責任を負わないものとします。 ご利用の際は各自の自己責任で使用してください。
* Special Thanks
  * プログラム内のアイコンは、[ICOON MONO](https://icooon-mono.com/)で公開されているアイコン素材を使用させていただきました。
