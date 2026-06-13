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

#include "uicheckbox.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#ifdef _WIN32
# include "uipalette.h"
#endif


UICheckBox::UICheckBox(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UICheckBox::~UICheckBox()
{

}

void UICheckBox::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

bool UICheckBox::isChecked()
{
    return m_checked;
}

#ifdef _WIN32
bool UICheckBox::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        return false;
    }

    case WM_LBUTTONUP: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
            click();
        }
        return false;
    }

    case WM_MOUSEENTER: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
        }
        return false;
    }

    case WM_MOUSELEAVE: {
        if (m_mouse_entered) {
            m_mouse_entered = false;
        }
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Normal);
            repaint();
        }
        return false;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UICheckBox::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UICheckBox::onPaint(const RECT&)
{
    engine()->DrawCheckBox(m_text, m_hFont, m_check_rc, m_checked);
#ifdef __linux__
    if (m_restrictedClickArea)
        updateInputRegion(m_check_rc);
#endif
}

void UICheckBox::click()
{
    m_checked = !m_checked;
    update();
    UIAbstractButton::click();
}
