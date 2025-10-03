#include "uiapplication.h"
#include "mainwindow.h"
#include "uistyle.h"
#include <locale>
#include "resource.h"
#include "utils.h"
#include "uiutils.h"
#include "translator.h"
#include "../desktop-apps/win-linux/src/defines.h"
#include "../desktop-apps/win-linux/src/prop/defines_p.h"


static const WCHAR pVersion[] = _T("Application version:\n" VER_FILEVERSION_STR);

int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    int num_args = 0;
    if (LPTSTR *args = CommandLineToArgvW(lpCmdLine, &num_args)) {
        NS_Utils::parseCmdArgs(num_args, args);
        LocalFree(args);
    }
    if (NS_Utils::cmdArgContains(_T("--log"))) {
        NS_Logger::AllowWriteLog();
        NS_Logger::WriteLog(pVersion);
    }

    std::locale::global(std::locale(""));
    LCID lcid = MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT);
    Translator::instance().init(lcid, IDT_TRANSLATIONS);
    HANDLE hMutex = CreateMutex(NULL, FALSE, _T(VER_PRODUCTNAME_STR));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        NS_Utils::ShowMessage(_TR(MSG_ERR_ALREADY_RUNNING));
        return 0;
    }

    if (UIUtils::winVersion() < UIUtils::Win7) {
        wstring msg(_TR(MSG_ERR_SYSTEM));
        NS_Utils::Replace(msg, L"%1", _TR(CAPTION));
        NS_Utils::ShowMessage(msg);
        CloseHandle(hMutex);
        return 0;
    }

    if (!NS_Utils::checkAndWaitForAppClosure()) {
        CloseHandle(hMutex);
        return 0;
    }

    wstring path, arch;
    bool app_installed = NS_Utils::IsAppInstalled(path, &arch);

    UIApplication app(hInst, lpCmdLine, nCmdShow);
    app.style()->loadThemesFromResource(IDT_THEMES);
    app.style()->loadStylesFromResource(IDT_STYLES);
    app.style()->setDefaultTheme(_T("Light"));
    app.setFont(L"Segoe UI", 9.5);
    if (NS_Utils::IsRtlLanguage(lcid))
        app.setLayoutDirection(UIApplication::RightToLeft);

    RECT rc{0, 0, WINDOW_SIZE.width, WINDOW_SIZE.height};
    double screenDpi = UIUtils::screenDpiAtRect(rc);
    int scrWidth = GetSystemMetrics(SM_CXSCREEN);
    int scrHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (scrWidth - WINDOW_SIZE.width * screenDpi) / 2;
    int y = (scrHeight - WINDOW_SIZE.height * screenDpi) / 2;
    MainWindow w(nullptr, Rect(x, y, WINDOW_SIZE.width * screenDpi, WINDOW_SIZE.height * screenDpi));
    w.setMinimumSize(WINDOW_SIZE.width * screenDpi, WINDOW_SIZE.height * screenDpi);
    w.setResizable(false);
    w.onAboutToDestroy([&app]() {
        app.exit(0);
    });
    if (!app_installed)
        w.initInstallationMode();
    else
        w.initControlMode(arch);
    w.showAll();
    int exit_code = app.exec();
    CloseHandle(hMutex);
    return exit_code;
}
