using System;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Threading;

namespace PowerSchemeUtility
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        const string CommandProcessName = "PowerProfCmd.exe";

        System.Threading.Mutex applicationMutex = null;
        System.Windows.Forms.NotifyIcon notifyIcon;
        DispatcherTimer rotateTimer = null;

        bool backgroundApplicationModeSetting = false;
        bool showWindowModeSetting = true;
        bool changeModeAfterExitSetting = false;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public MainWindow()
        {
            // 多重起動をチェックする
            bool createdNew = false;
            this.applicationMutex = new System.Threading.Mutex(true, Assembly.GetExecutingAssembly().ManifestModule.Name, out createdNew);

            // ミューテックスの初期使用権が取得出来なかった場合は、すでに起動中のプロセスが存在するので終了する
            if (createdNew == false)
            {
                System.Windows.MessageBox.Show($"すでに起動中のため、終了します.", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                this.Close();
            }

            // 常駐アプリケーションとして起動するかの設定値を取得する
            this.backgroundApplicationModeSetting = false;
            if (this.LoadConfigulationNumeric("RunningAtBackground", 0) == 1)
            {
                this.backgroundApplicationModeSetting = true;
            }

            // 常駐アプリケーションとして起動した際のウィンドウ表示設定の設定値を取得する
            this.showWindowModeSetting = true;
            if (this.LoadConfigulationNumeric("ShowWindow", 1) == 0)
            {
                this.showWindowModeSetting = false;
            }

            // モードを変更したら終了するかの設定値を取得する
            this.changeModeAfterExitSetting = false;
            if (this.LoadConfigulationNumeric("ChangeModeAfterExit", 0) == 1)
            {
                this.changeModeAfterExitSetting = true;
            }

            // PowerProfCmdが正常実行できるか調べる
            try
            {
                var result = this.ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), string.Empty);
                if (result.Item1 != 0)
                {
                    System.Windows.MessageBox.Show($"PowerProfCmd.exeの結果が異常です.\nランタイムが不足していないか等、システム環境を再確認してください.\n\n{result.Item2}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show($"PowerProfCmd.exeが実行できません.\n想定外エラーです.\n\n{ex.ToString()}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                this.Close();
            }

            // タスクバーに登録する
            if (this.backgroundApplicationModeSetting == true)
            {
                this.notifyIcon = new NotifyIcon();
                this.notifyIcon.Text = this.Title;
                this.notifyIcon.Icon = new System.Drawing.Icon(System.Windows.Application.GetResourceStream(new Uri("app_icon.ico", UriKind.Relative)).Stream);
                this.notifyIcon.Visible = true;
                this.notifyIcon.MouseClick += (_, e) =>
                {
                    // タスクバーのアイコンを左クリックした際にウィンドウを表示する
                    if (e.Button == MouseButtons.Left)
                    {
                        this.Visibility = Visibility.Visible;
                    }
                };

                // 右クリックメニューを登録する
                var contextMenu = new System.Windows.Forms.ContextMenuStrip();
                contextMenu.Items.Add("常駐終了", null, (_, __) => { this.ForceExit(); });
                this.notifyIcon.ContextMenuStrip = contextMenu;
            }

            this.Initialized += (_, __) =>
            {
                // 現在の状態を表示するために、定期的にプロセスを実行するためのタイマ
                this.rotateTimer = new DispatcherTimer(DispatcherPriority.Normal, this.Dispatcher);
                this.rotateTimer.Interval = new System.TimeSpan(0, 0, 0, 0, 1000);
                this.rotateTimer.Tick += this.onRotateTimerTick;
                this.rotateTimer.Start();

                // 常駐アプリケーションとして起動した際のウィンドウの非表示設定を行う
                if (this.backgroundApplicationModeSetting == true && this.showWindowModeSetting == false)
                {
                    this.Visibility = Visibility.Hidden;
                }
            };

            InitializeComponent();
        }

        /// <summary>
        /// 現在の実行ディレクトリのパスを取得します
        /// </summary>
        public string ExecuteFilePath
        {
            get
            {
                return Directory.GetParent(Assembly.GetExecutingAssembly().Location).FullName;
            }
        }

        /// <summary>
        /// 定期実行タイマが発火した際のイベントハンドラ
        /// </summary>
        /// <param name="sender">イベント発生元オブジェクト</param>
        /// <param name="e">イベント内容</param>
        private async void onRotateTimerTick(object sender, System.EventArgs e)
        {
            Tuple<int, string> result = null;

            // 画面を表示していない場合は、何もしない
            if (this.Visibility == Visibility.Hidden)
            {
                return;
            }

            // 現在の状態を取得する
            await Task.Run(() =>
            {
                result = this.ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), "/GetActiveSchemeText");
            });

            // 実行結果が異常な場合は、エラーメッセージを表示して終了する
            if (result.Item1 != 0)
            {
                System.Windows.MessageBox.Show($"現在の状態を取得することができません.\n\n{result.Item2}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                this.Close();
            }

            // 現在の状態を表示する
            this.textBlockCurrentState.Text = result.Item2;

            return;
        }

        /// <summary>
        /// ボタンクリック時のイベントハンドラ
        /// </summary>
        /// <param name="sender">イベント発生元オブジェクト</param>
        /// <param name="e">イベント内容</param>
        private void onSchemeButtonClick(object sender, RoutedEventArgs e)
        {
            int schemeId = -1;

            if (sender == this.buttonSelectBetterBatteryLife)
            {
                schemeId = 10;
            }
            else if (sender == this.buttonSelectBetterBalance)
            {
                schemeId = 20;
            }
            else if (sender == this.buttonSelectMaxPerformance)
            {
                schemeId = 40;
            }

            try
            {
                if (schemeId <= 0)
                {
                    return;
                }

                // 現在のモードを非表示にする
                this.textBlockCurrentState.Text = string.Empty;

                // モード変更処理の呼び出し
                var result = this.ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), $"/SetActiveSchemeId {schemeId}");
                if (result.Item1 != 0)
                {
                    System.Windows.MessageBox.Show($"PowerProfCmd.exeの結果が異常です.\n\n{result.Item2}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                    this.Close();
                }

                // モードを変更したら終了する設定の場合は、ウィンドウを閉じる
                if (this.changeModeAfterExitSetting == true)
                {
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show($"PowerProfCmd.exeが実行できません.\n想定外エラーです.\n\n{ex.ToString()}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                this.Close();
            }
        }

        /// <summary>
        /// 閉じるボタンクリック時のイベントハンドラ
        /// </summary>
        /// <param name="sender">イベント発生元オブジェクト</param>
        /// <param name="e">イベント内容</param>
        private void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// ハイパーリンクがクリックされた際のイベントハンドラ
        /// </summary>
        /// <param name="sender">イベント発生元オブジェクト</param>
        /// <param name="e">イベント内容</param>
        private void onHyperLinkClick(object sender, RoutedEventArgs e)
        {
            var result = ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), string.Empty);
            System.Windows.MessageBox.Show($"{result.Item2.ToString()}", this.Title, MessageBoxButton.OK, MessageBoxImage.Information);

            return;
        }

        /// <summary>
        /// ウィンドウクローズ直前のイベントハンドラ
        /// </summary>
        /// <param name="sender">イベント発生元オブジェクト</param>
        /// <param name="e">イベント内容</param>
        private void onWindowClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (this.backgroundApplicationModeSetting == true)
            {
                // 常駐アプリケーションとして動作させる場合は、ウィンドウを閉じるだけで終了しない
                e.Cancel = true;
                this.Visibility = Visibility.Hidden;
            }
            else
            {
                // 終了処理を行う
                this.ForceExit();
            }

            return;
        }

        /// <summary>
        /// 終了処理を行います
        /// </summary>
        private void ForceExit()
        {
            // ミューテックスを開放
            if (this.applicationMutex != null)
            {
                this.applicationMutex.Close();
                this.applicationMutex.Dispose();

                this.applicationMutex = null;
            }

            // タスクバーのアイコンを削除
            if (this.notifyIcon != null)
            {
                this.notifyIcon.Dispose();

                this.notifyIcon = null;
            }

            App.Current.Shutdown();
        }

        /// <summary>
        /// 数値で構成される設定値を取得します
        /// </summary>
        /// <param name="keyName">設定キー名</param>
        /// <param name="defaultValue">デフォルト値</param>
        /// <returns>設定値</returns>
        private int LoadConfigulationNumeric(string keyName, int defaultValue)
        {
            int result = defaultValue;

            try
            {
                var config = ConfigurationManager.AppSettings[keyName];
                if (config != null)
                {
                    result = int.Parse(config);
                }
            }
            catch
            {
                result = defaultValue;
            }

            return result;
        }

        /// <summary>
        /// プロセスを実行します
        /// </summary>
        /// <param name="path"></param>
        /// <param name="arg"></param>
        /// <returns></returns>
        private Tuple<int, string> ExecuteProcess(string path, string arg = null)
        {
            ProcessStartInfo processInfo = new ProcessStartInfo
            {
                FileName = path,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                UseShellExecute = false
            };

            if (arg != null)
            {
                processInfo.Arguments = arg;
            }

            Process process = Process.Start(processInfo);
            process.WaitForExit();

            return new Tuple<int, string>(process.ExitCode, process.StandardOutput.ReadToEnd());
        }
    }
}
