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

#ifndef UITHREAD_H
#define UITHREAD_H

#include "uiwidget.h"
#ifdef __linux__
# include "uiapplication.h"
#endif


namespace UIThread
{
    template<typename Widget, typename Fn, typename... Args>
    typename std::enable_if<std::is_member_function_pointer<Fn>::value>::type
    invoke(Widget *wgt, Fn&& fn, Args&&... args) {
        auto func = new std::function<void()>(std::bind(std::forward<Fn>(fn), wgt, std::forward<Args>(args)...));
#ifdef _WIN32
        PostMessage(wgt->platformWindow(), WM_INVOKEMETHOD, (WPARAM)func, 0);
#else
        UIApplication::postEvent(wgt->platformWindow(), GDK_INVOKEMETHOD, (void*)func);
#endif
    }   // NOLINT

    template<typename Widget, typename Fn>
    typename std::enable_if<!std::is_member_function_pointer<Fn>::value>::type
    invoke(Widget *wgt, Fn&& fn) {
        auto func = new std::function<void()>(std::forward<Fn>(fn));
#ifdef _WIN32
        PostMessage(wgt->platformWindow(), WM_INVOKEMETHOD, (WPARAM)func, 0);
#else
        UIApplication::postEvent(wgt->platformWindow(), GDK_INVOKEMETHOD, (void*)func);
#endif
    }   // NOLINT
};

#endif // UITHREAD_H
