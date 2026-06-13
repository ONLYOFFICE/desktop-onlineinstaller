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

#include "uitooltiphandler.h"
#include "uifontmetrics.h"
#include "uitooltip.h"
#include "uiwidget.h"
#include "uicursor.h"
#include "uitimer.h"

UIToolTipHandler::UIToolTipHandler(UIWidget* parent) :
    m_parent(parent),
    m_tooltip(nullptr),
    m_checkTimer(new UITimer),
    m_lastCursorPos(Point(-1, -1)),
    m_tooltipTickCounter(0),
    m_tooltipSet(false)
{
    m_checkTimer->onTimeout([this] {
        onToolTipCheck();
    });
}

UIToolTipHandler::~UIToolTipHandler()
{
    hideToolTip();
    delete m_checkTimer; m_checkTimer = nullptr;
}

void UIToolTipHandler::setToolTipText(const tstring &text) noexcept
{
    m_tooltipText = text;
    m_tooltipSet = !text.empty();
}

void UIToolTipHandler::handleMouseMove()
{
    if (m_tooltipSet && !m_checkTimer->isActive()) {
#ifdef _WIN32
        int x = 0, y = 0;
        UICursor::globalPos(x, y);
        if (x != m_lastCursorPos.x || y != m_lastCursorPos.y)
            m_checkTimer->start(1000);
#else
        m_checkTimer->start(1000);
#endif
    }
}

void UIToolTipHandler::skipToolTip()
{
    if (!m_tooltip && m_checkTimer->isActive())
        m_checkTimer->stop();
}

void UIToolTipHandler::hideToolTip()
{
    m_checkTimer->stop();
    if (m_tooltip) {
        m_tooltip->close();
        m_tooltip = nullptr;
    }
    m_tooltipTickCounter = 0;
}

void UIToolTipHandler::onToolTipCheck()
{    
    if (!m_parent->underMouse()) {
        hideToolTip();
        return;
    }

    if (m_tooltip) {
        if (m_tooltipTickCounter > 2) {
            hideToolTip();
            return;
        }
    } else
    if (!m_tooltipText.empty()) {
        int x = 0, y = 0;
        int width = 0, height = 0;
        UIFontMetrics fm;
        fm.textSize(m_parent, m_parent->m_hFont, m_tooltipText, width, height);
        double dpi = m_parent->dpiRatio();
        width += 20 * dpi;
        height += 10 * dpi;
        UICursor::globalPos(x, y);
        m_lastCursorPos = Point(x, y);
        m_tooltip = new UIToolTip(m_parent->topLevelWidget(), Rect(x + 10 * dpi, y + 10 * dpi, width, height));
        m_tooltip->setObjectGroupId(_T("ToolTip"));
        m_tooltip->setText(m_tooltipText);
        m_tooltip->show();
        m_tooltip->onClose([=](bool *accept) {
            *accept = true;
            if (m_checkTimer->isActive())
                m_checkTimer->stop();
            m_tooltipTickCounter = 0;
            m_tooltip = nullptr;
        });
    }
    ++m_tooltipTickCounter;
}
