/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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
    app.style()->setDefaultTheme(UIUtils::shouldAppsUseDarkMode() ? _T("Dark") : _T("Light"));
    app.setFont({DEFAULT_FONT_NAME, 9.5});
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
        w.initControlMode(path, arch);
    w.showAll();
    int exit_code = app.exec();
    CloseHandle(hMutex);
    return exit_code;
}
