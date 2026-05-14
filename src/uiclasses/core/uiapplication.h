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

#ifndef UIAPPLICATION_H
#define UIAPPLICATION_H

#include "uidefines.h"
#include "uiobject.h"
#include "uicommon.h"
#ifdef _WIN32
# include <Windows.h>
#else
# include <gtk/gtk.h>
#endif


class UIStyle;
class UIWidget;
class DECL_VISUALUI UIApplication : public UIObject
{
public:
#ifdef _WIN32
    explicit UIApplication(HINSTANCE hInstance, PWSTR cmdline, int cmdshow);
#else
    explicit UIApplication(int argc, char *argv[]);
#endif
    UIApplication(const UIApplication&) = delete;
    ~UIApplication();

    enum LayoutDirection : unsigned char {
        LeftToRight = 0,
        RightToLeft
    };

    UIApplication& operator=(const UIApplication&) = delete;
    static UIApplication *instance();
#ifdef _WIN32
    HINSTANCE moduleHandle();
#else
    static void postEvent(GtkWidget*, uint event_type, void *param);
    static bool sendEvent(GtkWidget*, uint event_type, void *param);
#endif
    void setLayoutDirection(LayoutDirection);
    void setFont(const FontInfo &fontInfo);
    FontInfo font() const;
    LayoutDirection layoutDirection() const;
    UIStyle* style();

    int exec();
    void exit(int);

private:
    friend class UIWidget;
    UIApplication();

    void registerWidget(UIWidget*, ObjectType, const Rect &rc);

    static UIApplication *inst;
    class UIApplicationPrivate;
    UIApplicationPrivate *d_ptr;
};

#endif // UIAPPLICATION_H
