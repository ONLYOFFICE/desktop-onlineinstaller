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

#include "uifontmetrics.h"
#include "uiwidget.h"
#ifdef _WIN32
# include <gdiplus.h>
#endif

UIFontMetrics::UIFontMetrics()
{

}

UIFontMetrics::~UIFontMetrics()
{

}

void UIFontMetrics::textSize(UIWidget *wgt, PlatformFont hFont, const tstring &text, int &width, int &height) const
{
    PlatformWindow hWnd = wgt->platformWindow();
#ifdef _WIN32
    HDC hdc = GetDC(hWnd);
    Gdiplus::Graphics gr(hdc);
    LOGFONTW logFont = {0};
    GetObject(hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(hdc, &logFont);
    Gdiplus::RectF lutRc, boxRc;
    gr.MeasureString(text.c_str(), text.length(), &font, lutRc, &boxRc);
    ReleaseDC(hWnd, hdc);
    width = boxRc.Width;
    height = boxRc.Height;
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(hWnd, text.c_str());
    pango_layout_set_font_description(lut, hFont);
    pango_layout_get_size(lut, &width, &height);
    width /= PANGO_SCALE;
    height /= PANGO_SCALE;
    g_object_unref(lut);
#endif
}
