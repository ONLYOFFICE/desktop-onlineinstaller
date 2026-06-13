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

#include "uiabstractbutton.h"
#include "uifontmetrics.h"
#include "uitooltiphandler.h"
#include "uipalette.h"
#include "uimetrics.h"
#include "uistyle.h"
#ifdef _WIN32
# include <windowsx.h>
#else
# include "uiapplication.h"
#endif


UIAbstractButton::UIAbstractButton(UIWidget *parent, const tstring &text) :
    UIWidget(parent, ObjectType::WidgetType),
    m_text(text),
#ifndef VISUALUI_SIMPLIFIED
    m_tooltipHandler(nullptr),
#endif
    m_checked(false),
    m_restrictedClickArea(false)
{
#ifdef _WIN32
    DWORD dwStyle = ::GetClassLong(m_hWindow, GCL_STYLE);
    dwStyle &= ~CS_DBLCLKS;
    ::SetClassLong(m_hWindow, GCL_STYLE, dwStyle);
#endif
}

UIAbstractButton::~UIAbstractButton()
{
#ifndef VISUALUI_SIMPLIFIED
    if (m_tooltipHandler) {
        delete m_tooltipHandler; m_tooltipHandler = nullptr;
    }
#endif
}

void UIAbstractButton::setText(const tstring &text) noexcept
{
    m_text = text;
    update();
}

#ifndef VISUALUI_SIMPLIFIED
void UIAbstractButton::setToolTip(const tstring &text) noexcept
{
    if (!m_tooltipHandler)
        m_tooltipHandler = new UIToolTipHandler(this);
    m_tooltipHandler->setToolTipText(text);
}
#endif

tstring UIAbstractButton::text() noexcept
{
    return m_text;
}

void UIAbstractButton::restrictClickArea(bool restrict) noexcept
{
    m_restrictedClickArea = restrict;
}

void UIAbstractButton::adjustSizeBasedOnContent()
{
    int width = 0, height = 0;
    const Metrics *mtr = metrics();
    UIFontMetrics fm;
    fm.textSize(this, m_hFont, m_text, width, height);
    width /= m_dpi_ratio;
    height /= m_dpi_ratio;
    int w = width + 2 * mtr->value(Metrics::IconWidth) + mtr->value(Metrics::TextMarginLeft) + mtr->value(Metrics::TextMarginRight);
#ifdef _WIN32
    int h = max(height + mtr->value(Metrics::TextMarginTop) + mtr->value(Metrics::TextMarginBottom), mtr->value(Metrics::IconHeight));
#else
    int h = std::max(height + mtr->value(Metrics::TextMarginTop) + mtr->value(Metrics::TextMarginBottom), mtr->value(Metrics::IconHeight));
#endif
    setBaseSize(w, h);
}

int UIAbstractButton::onClick(const FnVoidVoid &callback)
{
    m_click_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

void UIAbstractButton::disconnect(int connectionId)
{
    auto it = m_click_callbacks.find(connectionId);
    if (it != m_click_callbacks.end())
        m_click_callbacks.erase(it);
}

#ifdef _WIN32
bool UIAbstractButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_NCHITTEST: {
        if (!m_restrictedClickArea)
            break;
        *result = checkInputRegion(lParam, m_check_rc);
        return true;
    }

    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
            click();
        }
        break;
    }

    case WM_MOUSEENTER: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();

        }
        break;
    }

    case WM_MOUSELEAVE: {
        if (!m_disabled) {
#ifndef VISUALUI_SIMPLIFIED
            if (m_tooltipHandler)
                m_tooltipHandler->skipToolTip();
#endif
            palette()->setCurrentState(Palette::Normal);
            repaint();
        }
        break;
    }

    case WM_MOUSEMOVE: {
        if (!m_disabled) {
#ifndef VISUALUI_SIMPLIFIED
            if (m_tooltipHandler)
                m_tooltipHandler->handleMouseMove();
#endif
        }
        break;
    }

    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIAbstractButton::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_BUTTON_PRESS: {
        UIWidget::event(ev_type, param);
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        return true;
    }

    case GDK_BUTTON_RELEASE: {
        UIWidget::event(ev_type, param);
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
            click();
        }
        return true;
    }

    case GDK_ENTER_NOTIFY: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
        }
        break;
    }

    case GDK_LEAVE_NOTIFY: {
        if (!m_disabled) {
#ifndef VISUALUI_SIMPLIFIED
            if (m_tooltipHandler)
                m_tooltipHandler->skipToolTip();
#endif
            palette()->setCurrentState(Palette::Normal);
            repaint();
        }
        break;
    }

    case GDK_MOTION_NOTIFY: {
        if (!m_disabled) {
#ifndef VISUALUI_SIMPLIFIED
            if (m_tooltipHandler)
                m_tooltipHandler->handleMouseMove();
#endif
        }
        break;
    }

    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}
#endif

void UIAbstractButton::click()
{
    if (underMouse()) {
#ifndef VISUALUI_SIMPLIFIED
        if (m_tooltipHandler)
            m_tooltipHandler->skipToolTip();
#endif
        for (auto it = m_click_callbacks.begin(); it != m_click_callbacks.end(); it++) {
            if (it->second)
                (it->second)();
        }        
    }
}
