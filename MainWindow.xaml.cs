using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Threading;

namespace PowerSchemeUtility
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        const string CommandProcessName = "PowerProfCmd.exe";
        DispatcherTimer rotateTimer = null;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public MainWindow()
        {
            // PowerProfCmdが正常実行できるか調べる
            try
            {
                var result = this.ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), string.Empty);
                if (result.Item1 != 0)
                {
                    MessageBox.Show($"PowerProfCmd.exeの結果が異常です.\nランタイムが不足していないか等、システム環境を再確認してください.\n\n{result.Item2}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"PowerProfCmd.exeが実行できません.\n想定外エラーです.\n\n{ex.ToString()}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                this.Close();
            }

            this.Initialized += (_, __) =>
            {
                // 現在の状態を表示するために、定期的にプロセスを実行するためのタイマ
                this.rotateTimer = new DispatcherTimer(DispatcherPriority.Normal, this.Dispatcher);
                this.rotateTimer.Interval = new System.TimeSpan(0, 0, 0, 0, 1000);
                this.rotateTimer.Tick += this.onRotateTimerTick;
                this.rotateTimer.Start();
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
        private void onRotateTimerTick(object sender, System.EventArgs e)
        {
            var result = this.ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), "/GetActiveSchemeText");
            if (result.Item1 != 0)
            {
                MessageBox.Show($"現在の状態を取得することができません.\n\n{result.Item2}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
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

                var result = this.ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), $"/SetActiveSchemeId {schemeId}");
                if (result.Item1 != 0)
                {
                    MessageBox.Show($"PowerProfCmd.exeの結果が異常です.\n\n{result.Item2}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"PowerProfCmd.exeが実行できません.\n想定外エラーです.\n\n{ex.ToString()}", this.Title, MessageBoxButton.OK, MessageBoxImage.Error);
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
            Environment.Exit(0);
        }

        /// <summary>
        /// ハイパーリンクがクリックされた際のイベントハンドラ
        /// </summary>
        /// <param name="sender">イベント発生元オブジェクト</param>
        /// <param name="e">イベント内容</param>
        private void onHyperLinkClick(object sender, RoutedEventArgs e)
        {
            var result = ExecuteProcess(Path.Combine(this.ExecuteFilePath, MainWindow.CommandProcessName), string.Empty);
            MessageBox.Show($"{result.Item2.ToString()}", this.Title, MessageBoxButton.OK, MessageBoxImage.Information);

            return;
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
