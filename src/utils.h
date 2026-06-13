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

#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include <string>
#include <vector>
#include <list>

using std::string;
using std::wstring;
using std::to_wstring;
using std::list;

#define DEFAULT_ERROR_MESSAGE _T("An error occurred:\n") + \
    wstring(_T(__FUNCTION__)) + _T(" Line: ") + to_wstring(__LINE__)
#define ADVANCED_ERROR_MESSAGE DEFAULT_ERROR_MESSAGE + \
    _T(" ") + NS_Utils::GetLastErrorAsString()


namespace NS_Utils
{
void parseCmdArgs(int argc, wchar_t *argv[]);
bool cmdArgContains(const wstring &param);
wstring cmdArgValue(const wstring &param);
wstring GetLastErrorAsString(DWORD errID = 0);
void ShowMessage(wstring str, bool showError = false);
int  ShowTaskDialog(HWND parent, const wstring &msg, PCWSTR icon);
bool IsRtlLanguage(unsigned long lcid);
bool IsWin64();
bool IsWinArm64();
bool IsAppInstalled(wstring &path, wstring *arch = nullptr);
bool IsCommunityEdition(const wstring &basePath);
bool checkAndWaitForAppClosure(HWND parent = nullptr);
void InstalledVerInfo(LPCWSTR value, wstring &name);
void Replace(wstring &str, const wstring &from, const wstring &to);
wstring MsiGetProperty(LPCWSTR prodCode, LPCWSTR propName);
wstring MsiProductCode(const wstring &prodName);
}

namespace NS_File
{
DWORD runProcess(const wstring &fileName, const wstring &args, bool runAsAdmin = false, bool wait = true);
// bool isProcessRunning(const wstring &fileName);
std::vector<wstring> findFilesByPattern(const wstring &path, const wstring &pattern);
bool readFile(const wstring &filePath, list<wstring> &linesList);
bool fileExists(const wstring &filePath);
bool removeFile(const wstring &filePath);
bool removeDirRecursively(const wstring &dir);
wstring fromNativeSeparators(const wstring &path);
wstring toNativeSeparators(const wstring &path);
wstring parentPath(const wstring &path);
wstring tempPath();
wstring appPath();
wstring appDataPath();
wstring generateTmpFileName(const wstring &ext);
wstring getFileHash(const wstring &fileName);
bool verifyEmbeddedSignature(const wstring &fileName);
}

namespace NS_Logger
{
void AllowWriteLog();
void WriteLog(const wstring &log, bool showMessage = false);
}

#endif // UTILS_H
