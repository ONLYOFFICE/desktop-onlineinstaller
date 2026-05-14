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

#include "uibutton.h"
#include "uidrawningengine.h"
#ifdef _WIN32
# include <windowsx.h>
# include "uipalette.h"
# include "uiutils.h"

static bool isArrangingAllowed() {
    BOOL arranging = FALSE;
    SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
    return (arranging == TRUE);
}
#else
#endif


UIButton::UIButton(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this),
    m_stockIcon(StockIcon::None)
#ifdef _WIN32
    , supportSnapLayouts(false),
    snapLayoutAllowed(false),
    snapLayoutTimerIsSet(false)
#else
#endif
{

}

UIButton::~UIButton()
{

}

void UIButton::setSupportSnapLayouts()
{
#ifdef _WIN32
    if (UIUtils::winVersion() > UIUtils::WinVer::Win10) {
        snapLayoutAllowed = isArrangingAllowed();
        supportSnapLayouts = true;
    }
#endif
}

void UIButton::setStockIcon(StockIcon stockIcon)
{
    m_stockIcon = stockIcon;
    update();
}

#ifdef _WIN32
bool UIButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_NCHITTEST: {
        if (supportSnapLayouts && snapLayoutAllowed) {
            if (!snapLayoutTimerIsSet) {
                snapLayoutTimerIsSet = true;
                palette()->setCurrentState(Palette::Hover);
                SetTimer(m_hWindow, SNAP_LAYOUTS_TIMER_ID, 100, NULL);
                repaint();
            }
            *result = HTMAXBUTTON;
            return true;
        }
        return false;
    }

    case WM_TIMER: {
        if (wParam == SNAP_LAYOUTS_TIMER_ID) {
            if (!underMouse()) {
                KillTimer(m_hWindow, wParam);
                snapLayoutTimerIsSet = false;
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case WM_CAPTURECHANGED: {
        if (UIUtils::winVersion() > UIUtils::WinVer::Win10) {
            click();
        }
        break;
    }

    case WM_SETTINGCHANGE_NOTIFY: {
        if (wParam == SPI_SETWINARRANGING)
            snapLayoutAllowed = isArrangingAllowed();
        break;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UIButton::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UIButton::onPaint(const RECT &rc)
{
    UIDrawingEngine *de = engine();
#ifdef _WIN32
    if (m_hIcon)
        de->DrawIcon(m_hIcon);
    if (m_hBmp)
        de->DrawImage(m_hBmp);
    if (m_hEmf)
        de->DrawEmfIcon(m_hEmf);
#else
    if (m_hBmp)
        de->DrawIcon(m_hBmp);
    if (m_hSvg)
        de->DrawSvgIcon(m_hSvg);
#endif
    if (!m_text.empty())
        de->DrawString(rc, m_text, m_hFont);

    if (m_stockIcon == StockIcon::CloseIcon)
        de->DrawStockCloseIcon();
    else
    if (m_stockIcon == StockIcon::RestoreIcon)
        de->DrawStockRestoreIcon();
    else
    if (m_stockIcon == StockIcon::MinimizeIcon)
        de->DrawStockMinimizeIcon();
    else
    if (m_stockIcon == StockIcon::MaximizeIcon)
        de->DrawStockMaximizeIcon();
}
