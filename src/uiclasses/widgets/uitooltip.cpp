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

#include "uitooltip.h"
#include "uidrawningengine.h"
#include "uiopacityanimation.h"


UIToolTip::UIToolTip(UIWidget *parent, const Rect &rc) :
    UIAbstractPopup(parent, rc),
    m_animation(new UIOpacityAnimation(this))
{
#ifdef _WIN32
    SetProp(m_hWindow, L"BlockActivation", (HANDLE)1);
#else
    gtk_widget_set_can_focus(m_hWindow, FALSE);
    gtk_window_set_accept_focus(GTK_WINDOW(m_hWindow), FALSE);
    gtk_window_set_focus_on_map(GTK_WINDOW(m_hWindow), FALSE);
#endif
}

UIToolTip::~UIToolTip()
{
    delete m_animation; m_animation = nullptr;
}

void UIToolTip::setText(const tstring &text)
{
    m_text = text;
}

void UIToolTip::close()
{
    m_animation->startFadeOut();
}

#ifdef _WIN32
bool UIToolTip::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            m_animation->startFadeIn();
        }
        break;
    }

    case WM_NCHITTEST: {
        *result = HTTRANSPARENT;
        return true;
    }

    case WM_TOPLEVEL_ACTIVATION_NOTIFY:
    case WM_TOPLEVEL_BUTTONDOWN_NOTIFY: {
        close();
        return true;
    }

    case WM_NCDESTROY: {
        RemoveProp(m_hWindow, L"BlockActivation");
        break;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(msg, wParam, lParam, result);
}

void UIToolTip::onPaintLayered(const RECT &rc, BYTE *opacity)
{
    *opacity = m_animation->opacity();
    if (!m_text.empty())
        engine()->DrawString(rc, m_text, m_hFont);
}
#else
bool UIToolTip::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_HOOKED_MAP_AFTER: {
        if (m_animation->isFadingOut()) {
            UIWidget::close();
            return false;
        }
        m_animation->startFadeIn();
        return false;
    }

    case GDK_TOPLEVEL_ACTIVATION_NOTIFY:
    case GDK_TOPLEVEL_BUTTONDOWN_NOTIFY: {
        gtk_widget_hide(m_hWindow); // Prevent input area from being restricted when modal window appears
        UIWidget::close();
        return true;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(ev_type, param);
}

void UIToolTip::onPaint(const RECT &rc)
{
    if (!m_text.empty())
        engine()->DrawString(rc, m_text, m_hFont);

    updateInputRegion(Rect());
}
#endif
