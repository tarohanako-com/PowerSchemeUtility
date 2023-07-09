// PowerProfCmd.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#if defined(UNICODE) || defined(_UNICODE)
# define tout std::wcout
# define tin std::wcin
# define terr std::wcerr
# define tlog std::wclog
#else
# define tout std::cout
# define tin std::cin
# define terr std::cerr
# define tlog std::clog
#endif
// ------------------------------------------------------------
// ヘッダファイル等のインクルード
// ------------------------------------------------------------
// スマートポインタのヘッダをインクルード
#include <memory>

// 共通ヘッダをインクルード
#include "afx.h"
#include "shellapi.h"
#pragma comment(lib, "version.lib")

// Power系API呼び出し用のヘッダとライブラリ
#include "powersetting.h"
#include <debugapi.h>
#include <Windows.h>
#include <powrprof.h>
#pragma comment(lib, "powrprof.lib")

#include <iostream>

#include <initguid.h>
DEFINE_GUID(GUID_SCHEME_TYPE_PERFORMANCE_SCHEME, 0x381B4222, 0xF694, 0x41F0, 0x96, 0x85, 0xFF, 0x5B, 0xB2, 0x60, 0xDF, 0x2E);
DEFINE_GUID(GUID_BETTER_BATTERY_LIFE_SCHEME, 0x961CC777, 0x2547, 0x4F9D, 0x81, 0x74, 0x7D, 0x86, 0x18, 0x1b, 0x8A, 0x7A);
DEFINE_GUID(GUID_BALANCED_SCHEME, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
DEFINE_GUID(GUID_MAX_PERFORMANCE_SCHEME, 0xDED574B5, 0x45A0, 0x4F42, 0x87, 0x37, 0x46, 0x34, 0x5C, 0x09, 0xC2, 0x38);

enum enumSchemeTypes
{
	SCHEME_TYPE_MIN = 0,
    SCHEME_TYPE_UNKNOWN = SCHEME_TYPE_MIN,
    SCHEME_TYPE_BETTER_BATTERY_LIFE = 10,
	SCHEME_TYPE_BALANCED = 20,
    SCHEME_TYPE_PERFORMANCE = 30,
	SCHEME_TYPE_MAX_PERFORMANCE = 40,
	SCHEME_TYPE_MAX = SCHEME_TYPE_MAX_PERFORMANCE,
};

struct tagPowerSchemeInfo
{
	GUID guid;
    enumSchemeTypes type;
	TCHAR szName[256];
};

// ------------------------------------------------------------
// 関数プロトタイプ宣言
// ------------------------------------------------------------
size_t get_own_version(TCHAR* pszVersion, const int nSize);
void print_banner();
void print_usage();

// ------------------------------------------------------------
// 関数ポインタ宣言
// ------------------------------------------------------------
typedef DWORD(*fnPowerGetActualOverlayScheme)(GUID*);
typedef DWORD(*fnPowerGetEffectiveOverlayScheme)(GUID*);
typedef DWORD(*fnPowerSetActiveOverlayScheme)(GUID*);

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
HMODULE g_hPowrProfLibrary = NULL;
fnPowerGetActualOverlayScheme g_fnPowerGetActualOverlayScheme = NULL;
fnPowerGetEffectiveOverlayScheme g_fnPowerGetEffectiveOverlayScheme = NULL;
fnPowerSetActiveOverlayScheme g_fnPowerSetActiveOverlayScheme = NULL;

const TCHAR* g_szUnknownSchemeName = _T("パフォーマンス名を取得できません");
tagPowerSchemeInfo g_PowerSchemeDefinitions[] = {
	{ GUID_BETTER_BATTERY_LIFE_SCHEME, SCHEME_TYPE_BETTER_BATTERY_LIFE, _T("より良いバッテリー - トップクラスの電力効率") },
    { GUID_BALANCED_SCHEME, SCHEME_TYPE_BALANCED, _T("高パフォーマンス - バランス") },
	{ GUID_MAX_PERFORMANCE_SCHEME, SCHEME_TYPE_MAX_PERFORMANCE, _T("最も高いパフォーマンス - 最適なパフォーマンス") },
};

// ------------------------------------------------------------
// 処理関数
// ------------------------------------------------------------
/// <summary>
/// メイン関数
/// </summary>
/// <returns>実行結果</returns>
int main()
{
    int nRet = 0;
    int nArgsCount = 0;
    LPTSTR* szArgumentsArray = NULL;

    setlocale(LC_ALL, "Japanese");

    // ============================================================
    // 初期化処理
    // ============================================================
    // PowrProf.dllのロード
    g_hPowrProfLibrary = ::LoadLibrary(_T("PowrProf.dll"));
    if (g_hPowrProfLibrary == NULL)
    {
        // PowrProf.dllのロードに失敗した場合は、エラーを表示して終了
        print_banner();
        tout << _T("[ERROR] PowrProf.dllが初期化できません.") << std::endl;
        tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;

        nRet = -1;
    }

    // PowrProf.dllの関数ポインタを取得
    g_fnPowerGetActualOverlayScheme = (fnPowerGetActualOverlayScheme)::GetProcAddress(g_hPowrProfLibrary, "PowerGetActualOverlayScheme");
    if (g_fnPowerGetActualOverlayScheme == NULL)
    {
		// PowrProf.dllの関数ポインタの取得に失敗した場合は、エラーを表示して終了
		print_banner();
		tout << _T("[ERROR] PowrProf.dll -- PowerGetActualOverlayScheme()の関数ポインタの取得に失敗しました.") << std::endl;
        tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;

        nRet = -2;
        goto END_PROC;
	}

    g_fnPowerGetEffectiveOverlayScheme = (fnPowerGetEffectiveOverlayScheme)::GetProcAddress(g_hPowrProfLibrary, "PowerGetEffectiveOverlayScheme");
    if (g_fnPowerGetEffectiveOverlayScheme == NULL)
    {
        // PowrProf.dllの関数ポインタの取得に失敗した場合は、エラーを表示して終了
        print_banner();
        tout << _T("[ERROR] PowrProf.dll -- PowerGetEffectiveOverlayScheme()の関数ポインタの取得に失敗しました.") << std::endl;
        tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;

        nRet = -2;
        goto END_PROC;
    }

    g_fnPowerSetActiveOverlayScheme = (fnPowerSetActiveOverlayScheme)::GetProcAddress(g_hPowrProfLibrary, "PowerSetActiveOverlayScheme");
    if (g_fnPowerSetActiveOverlayScheme == NULL)
    {
        // PowrProf.dllの関数ポインタの取得に失敗した場合は、エラーを表示して終了
        print_banner();
        tout << _T("[ERROR] PowrProf.dll -- PowerSetActiveOverlayScheme()の関数ポインタの取得に失敗しました.") << std::endl;
        tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;

        nRet = -3;
        goto END_PROC;
    }
    
    // ============================================================
    // 実処理開始
    // ============================================================
    // コマンドライン引数の解析
    nArgsCount = 0;
    szArgumentsArray = ::CommandLineToArgvW(::GetCommandLine(), &nArgsCount);

    if (nArgsCount <= 1)
    {
        // コマンドラインが設定されていない場合は使い方を表示する
        print_banner();
        print_usage();

        nRet = 0;
        goto END_PROC;
    }

    // 指定されたコマンドごとに処理する
    if (_tcscmp(szArgumentsArray[1], _T("/GetActiveScheme")) == 0)
    {
        // 現在の設定を表示
        GUID tGuid;
        auto nFuncResult = g_fnPowerGetActualOverlayScheme(&tGuid);

        if (nFuncResult != 0)
        {
            // PowerGetActualOverlayScheme()の呼び出しに失敗した場合は、エラーを表示して終了
			print_banner();
			tout << _T("[ERROR] PowerGetActualOverlayScheme()の呼び出しに失敗しました. 復帰値: ") << std::dec << nFuncResult << std::endl;
            tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;

			nRet = -10;
			goto END_PROC;
        }

        // GUIDを文字列に変換
        TCHAR szGuidString[256];
        ::StringFromGUID2(tGuid, szGuidString, _countof(szGuidString));

        // GUIDを出力
        tout << szGuidString << std::endl;
        nRet = 0;

        goto END_PROC;
    }
    else if (_tcscmp(szArgumentsArray[1], _T("/GetActiveSchemeText")) == 0)
    {
        // 現在の設定を表示
        GUID tGuid;
        auto nFuncResult = g_fnPowerGetActualOverlayScheme(&tGuid);

        if (nFuncResult != 0)
        {
            // PowerGetActualOverlayScheme()の呼び出しに失敗した場合は、エラーを表示して終了
            print_banner();
            tout << _T("[ERROR] PowerGetActualOverlayScheme()の呼び出しに失敗しました. 復帰値: ") << std::dec << nFuncResult << std::endl;
            tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;

            nRet = -20;
            goto END_PROC;
        }

        // GUIDを文字列に変換
        TCHAR szGuidString[256];
        ::StringFromGUID2(tGuid, szGuidString, _countof(szGuidString));

        // 一致するGUIDの名称を取得
        CString szSchemeName = g_szUnknownSchemeName;
        for (auto& item : g_PowerSchemeDefinitions)
        {
            TCHAR szSchemeGuid[256];
            ::StringFromGUID2(item.guid, szSchemeGuid, _countof(szSchemeGuid));

            if (_tcscmp(szSchemeGuid, szGuidString) == 0)
            {
				// GUIDが一致した場合は、設定名を出力
                szSchemeName = item.szName;

                break;
			}
		}

        // スキーム名を出力
        tout << (LPCTSTR)szSchemeName << std::endl;
        nRet = 0;

        goto END_PROC;
    }
    else if (_tcscmp(szArgumentsArray[1], _T("/SetActiveSchemeId")) == 0)
    {
        // スキーマIDを取得
        if (nArgsCount <= 2)
        {
			// スキーマIDが指定されていない場合は、エラーを表示して終了
			print_banner();
			tout << _T("[ERROR] スキーマIDが指定されていません.") << std::endl;

            nRet = -30;
			goto END_PROC;
		}

        auto nSchemeId = _ttoi(szArgumentsArray[2]);

        // スキーマIDに該当する値を取得して、システムに設定する
        nRet = -31;     // 存在しないスキーマIDが指定された場合のエラー
        for (auto& item : g_PowerSchemeDefinitions)
        {
            if ((int)item.type == nSchemeId)
            {
                // 該当するスキーマIDが見つかったので、システムに設定する
                auto nFuncResult = g_fnPowerSetActiveOverlayScheme(&item.guid);

                if (nFuncResult == 0)
                {
                    nRet = 0;
                    tout << _T("[INFO] 正常終了しました.") << std::endl;
                }
                else
                {
                    nRet = -32;     // PowerSetActiveOverlayScheme()の呼び出しに失敗した場合のエラー
                    tout << _T("[ERROR] PowerSetActiveOverlayScheme()の呼び出しに失敗しました. 復帰値: ") << std::dec << nFuncResult << std::endl;
                    tout << _T("[ERROR] GetLastError: 0x") << std::dec << ::GetLastError() << _T("") << std::endl;
                }

                break;
            }
        }

        if (nRet == -31)
        {
            tout << _T("[ERROR] 存在しないスキーマIDです. 指定された値: ") << std::dec << nSchemeId << std::endl;
        }

        goto END_PROC;
    }

    // 一致するコマンド引数が存在しない場合は、使い方を表示
    print_banner();
    tout << _T("コマンドライン引数が認識できません!") << std::endl;
    print_usage();

    nRet = 0;

END_PROC:
    // PowrProf.dllのアンロード
    if (g_hPowrProfLibrary != NULL)
    {
		::FreeLibrary(g_hPowrProfLibrary);
        g_hPowrProfLibrary = NULL;
	}

	return nRet;
}

/// <summary>
/// 本プロセスのバージョン情報を文字列形式で取得します
/// </summary>
/// <param name="pszVersionString">バージョン情報の出力先</param>
/// <param name="nSize">バッファサイズ</param>
/// <returns>バージョン情報文字列形式サイズ</returns>
size_t get_own_version(TCHAR* pszVersionString, const size_t nSize)
{
    TCHAR szFileName[MAX_PATH + 1];
    DWORD dwNeedBufferSize = 0;

    // 本プロセスのファイル名を取得
    ::GetModuleFileName(NULL, szFileName, _countof(szFileName));

    // バージョン情報を取得用の領域サイズを取得
    dwNeedBufferSize = ::GetFileVersionInfoSize(szFileName, NULL);

    // バージョン情報を取得用の領域を確保
    std::unique_ptr<byte> pVersion(new byte[dwNeedBufferSize + 1]);

    // バージョン情報を取得
    if (::GetFileVersionInfo(szFileName, NULL, dwNeedBufferSize, pVersion.get()))
    {
        UINT nQueryLen = 0;
        CString sFileVersion;
        VS_FIXEDFILEINFO* ptFileInfo;

        ::VerQueryValue(pVersion.get(), _T("\\"), (void**)&ptFileInfo, &nQueryLen);

        // バージョン情報を文字列にフォーマット
        sFileVersion.Format(_T("%d.%d.%d.%d"),
            HIWORD(ptFileInfo->dwFileVersionMS),
            LOWORD(ptFileInfo->dwFileVersionMS),
            HIWORD(ptFileInfo->dwFileVersionLS),
            LOWORD(ptFileInfo->dwFileVersionLS));

        // バッファ容量が足りていればバージョン情報を返す
        if ((_tcslen((LPCTSTR)sFileVersion) + 1) < nSize)
        {
            _stprintf_s(pszVersionString, nSize, _T("%s"), (LPCTSTR)sFileVersion);
		}
        else
        {
            // バッファがあふれる場合は、空文字を返す
            pszVersionString[0] = _T('\0');
        }
    }
    else
    {
        // バージョン情報が取得できなかった場合は、空文字を返す
		pszVersionString[0] = _T('\0');
    }

    return _tcslen(pszVersionString) + 1;
}

/// <summary>
/// 本プロセスのバナーを出力します
/// </summary>
void print_banner()
{
    // バージョン情報を取得
    TCHAR szVersion[64];
    get_own_version(szVersion, _countof(szVersion));

    // バナーを出力
    tout << _T("-------------------------------------------------------------------") << std::endl;
    tout << _T(" PowerProfCmd.exe") << _T("  Ver: ") << szVersion << std::endl;
    tout << _T("-------------------------------------------------------------------") << std::endl;
    tout << _T("      (C)2023- taro-hanako device development, All right reserved. ") << std::endl;
    tout << _T("-------------------------------------------------------------------") << std::endl;
    tout << std::endl;
}

/// <summary>
/// 本プロセスの使用方法をコンソールに出力します
/// </summary>
void print_usage()
{
    // コマンドライン一覧
    tout << _T("* 使用可能なコマンドライン引数:") << std::endl;
    tout << _T("    /GetActiveScheme     : 現在のアクティブな設定をGUID形式で表示します.") << std::endl;
    tout << _T("    /GetActiveSchemeText : 現在のアクティブな設定を文字列形式で表示します.") << std::endl;
    tout << _T("    /SetActiveSchemeId   : スキーマIDを指定して、現在の設定を上書きします.") << std::endl;
    tout << std::endl;

    // 定義されているGUID
    tout << _T("* 定義されているスキーマ:") << std::endl;
    for (auto& item : g_PowerSchemeDefinitions)
    {
        TCHAR szSchemeGuid[256];
        ::StringFromGUID2(item.guid, szSchemeGuid, _countof(szSchemeGuid));

        tout << _T("    * ");
        tout << _T("スキーマ名: ") << item.szName;
        tout << _T(", スキーマID: ") << item.type;
        tout << _T(", GUID: ") << szSchemeGuid;
        tout << std::endl;
    }
    tout << std::endl;

    return;
}