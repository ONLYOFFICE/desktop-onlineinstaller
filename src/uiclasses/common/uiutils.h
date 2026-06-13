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

#ifndef UIUTILS_H
#define UIUTILS_H

#ifdef _WIN32
# include <Windows.h>
# include <gdiplus.h>
#else
# include <gio/gio.h>
#endif
#include "uidefines.h"


namespace UIUtils
{
#ifdef _WIN32
    enum WinVer : BYTE {
        Undef, WinXP, WinVista, Win7, Win8, Win8_1, Win10, Win11, WinFuture
    };
    WinVer DECL_VISUALUI winVersion() noexcept;
    std::wstring DECL_VISUALUI currentUserSID();
    DWORD DECL_VISUALUI regQueryDwordValue(HKEY rootKey, LPCWSTR subkey, LPCWSTR value, bool *success = nullptr);
    double DECL_VISUALUI screenDpiAtPoint(const POINT &pt);
    double DECL_VISUALUI screenDpiAtRect(const RECT &rc);
    bool DECL_VISUALUI shouldAppsUseDarkMode();
    void DECL_VISUALUI loadImageResource(Gdiplus::Bitmap* &hBmp, int id, LPCWSTR type);
    void DECL_VISUALUI loadEmfResource(Gdiplus::Metafile* &hBmp, int id, LPCWSTR type);
    void DECL_VISUALUI loadStringResource(tstring &str, int id);
#else
    enum class DesktopEnv : unsigned char {
        UNDEF, UNITY, GNOME, KDE, XFCE, CINNAMON, OTHER
    };
    enum class WindowServer : unsigned char {
        UNDEF, X11, WAYLAND, OTHER
    };
    DesktopEnv DECL_VISUALUI desktopEnv();
    WindowServer DECL_VISUALUI windowServer();
    void DECL_VISUALUI loadStringResource(tstring &str, GResource *res, const char *resourcePath);
#endif
    bool isAllocOnHeap(void *addr);
};

namespace UILocalization
{
#ifdef _WIN32
bool DECL_VISUALUI isRtlLanguage(unsigned long lcid);
#else
bool DECL_VISUALUI isRtlLanguage(const char *locale);
#endif
};

namespace UIUnicode
{
#ifdef _WIN32
std::wstring DECL_VISUALUI utf8ToWStr(const std::string &str);
#endif
size_t DECL_VISUALUI charLenAt(const tstring &str, size_t pos) noexcept;
size_t DECL_VISUALUI charLenBefore(const tstring &str, size_t pos) noexcept;
size_t DECL_VISUALUI charPrevPos(const tstring &str, size_t pos) noexcept;
size_t DECL_VISUALUI charNextPos(const tstring &str, size_t pos) noexcept;
};

#endif // UIUTILS_H
