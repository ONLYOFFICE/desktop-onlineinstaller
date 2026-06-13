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

#ifndef UIDEFINES_H
#define UIDEFINES_H

#include <functional>
#include <vector>
#include <string>

#if defined(_WIN32)
# define DECL_EXPORT __declspec(dllexport)
# define DECL_IMPORT __declspec(dllimport)
#else
# define DECL_EXPORT __attribute__((visibility("default")))
# define DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(VISUALUI_LIBRARY)
# define DECL_VISUALUI DECL_EXPORT
#elif defined(VISUALUI_STATIC)
# define DECL_VISUALUI
#else
# define DECL_VISUALUI DECL_IMPORT
#endif

#ifdef _WIN32
# include <tchar.h>
# define DEFAULT_FONT_NAME "Segoe UI"
  typedef std::wstring tstring;
#else
# define _T(x) x
# define DEFAULT_FONT_NAME "Noto Sans"
  typedef std::string tstring;
#endif

#ifdef _WIN32
# define SNAP_LAYOUTS_TIMER_ID    0x1f000000

# define WM_MOUSEENTER            (WM_APP + 1)
# define WM_INVOKEMETHOD          (WM_APP + 2)
# define WM_PARENT_ACTIVATION_NOTIFY   (WM_APP + 3)
# define WM_TOPLEVEL_ACTIVATION_NOTIFY (WM_APP + 4)
# define WM_CHILD_BUTTONDOWN_NOTIFY    (WM_APP + 5)
# define WM_TOPLEVEL_BUTTONDOWN_NOTIFY (WM_APP + 6)
# define WM_CHILD_KEYDOWN_NOTIFY       (WM_APP + 7)
# define WM_TOPLEVEL_KEYDONW_NOTIFY    (WM_APP + 8)
# define WM_SETTINGCHANGE_NOTIFY  (WM_APP + 9)
# define WM_DPICHANGED_NOTIFY     (WM_APP + 10)
# define WM_PAINT_LAYERED_CHILD   (WM_APP + 11)
# define WM_UPDATE_LAYERED_WINDOW (WM_APP + 12)
#else
# include <gdk/gdk.h>
# define GDK_INVOKEMETHOD         (GDK_EVENT_LAST + 1)
# define GDK_PARENT_ACTIVATION_NOTIFY   (GDK_EVENT_LAST + 2)
# define GDK_TOPLEVEL_ACTIVATION_NOTIFY (GDK_EVENT_LAST + 3)
# define GDK_CHILD_BUTTONDOWN_NOTIFY    (GDK_EVENT_LAST + 4)
# define GDK_TOPLEVEL_BUTTONDOWN_NOTIFY (GDK_EVENT_LAST + 5)
# define GDK_TOPLEVEL_KEYDONW_NOTIFY    (GDK_EVENT_LAST + 6)
# define GDK_HOOKED_DRAW          (GDK_EVENT_LAST + 7)
# define GDK_HOOKED_DESTROY       (GDK_EVENT_LAST + 8)
# define GDK_HOOKED_CONFIGURE_AFTER    (GDK_EVENT_LAST + 9)
# define GDK_HOOKED_FOCUS_CHANGE_AFTER (GDK_EVENT_LAST + 10)
# define GDK_HOOKED_WINDOW_STATE_AFTER (GDK_EVENT_LAST + 11)
# define GDK_HOOKED_SIZE_ALLOC         (GDK_EVENT_LAST + 12)
# define GDK_HOOKED_CONFIGURE     (GDK_EVENT_LAST + 13)
# define GDK_HOOKED_REALIZE       (GDK_EVENT_LAST + 14)
# define GDK_HOOKED_BUTTON_PRESS_AFTER (GDK_EVENT_LAST + 15)
# define GDK_HOOKED_DBLBUTTON_PRESS_AFTER (GDK_EVENT_LAST + 16)
# define GDK_HOOKED_MAP_AFTER          (GDK_EVENT_LAST + 17)
# define GDK_HOOKED_DROPFILES          (GDK_EVENT_LAST + 18)
#endif

typedef std::function<void(void)>     FnVoidVoid;
typedef std::function<void(bool)>     FnVoidBool;
typedef std::function<void(bool*)>    FnVoidBoolPtr;
typedef std::function<void(int)>      FnVoidInt;
typedef std::function<void(int,int)>  FnVoidIntInt;
typedef std::function<void(std::vector<tstring>)>  FnVoidVecStr;

#endif // UIDEFINES_H
