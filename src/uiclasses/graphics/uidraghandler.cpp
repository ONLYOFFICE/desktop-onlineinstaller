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

#include "uidraghandler.h"
#include "uiwidget.h"


UIDragHandler::UIDragHandler(UIWidget *target) :
    m_target(target),
    m_dragStart({0,0}),
    m_winStart({0,0}),
    m_dragging(false)
{
    m_hWindow = target->platformWindow();
#ifdef _WIN32
    DWORD style = GetWindowLong(m_hWindow, GWL_STYLE);
    SetWindowLong(m_hWindow, GWL_STYLE, style | WS_CLIPSIBLINGS);
#else
    m_parent = gtk_widget_get_parent(m_hWindow);
#endif
}

UIDragHandler::~UIDragHandler()
{

}

void UIDragHandler::handleButtonDownEvent(int x, int y)
{
    m_dragging = true;
    m_dragStart.x = x;
    m_dragStart.y = y;
    m_target->bringAboveSiblings();
    m_target->grabMouse();
#ifdef _WIN32
    ClientToScreen(m_hWindow, &m_dragStart);
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    m_winStart.x = rc.left;
    m_winStart.y = rc.top;
#else
    int child_x = 0, child_y = 0;
    if (m_parent) {
        gtk_widget_translate_coordinates(m_hWindow, m_parent, 0, 0, &child_x, &child_y);
    }
    m_winStart.x = child_x;
    m_winStart.y = child_y;
#endif
}

void UIDragHandler::handleButtonUpEvent()
{
    if (m_dragging) {
        m_dragging = false;
        m_target->ungrabMouse();
    }
}

void UIDragHandler::handleMouseMoveEvent(int x, int y)
{
    if (m_dragging) {
#ifdef _WIN32
        POINT pt;
        pt.x = x;
        pt.y = y;
        ClientToScreen(m_hWindow, &pt);
        int new_x = m_winStart.x + pt.x - m_dragStart.x;
        int new_y = m_winStart.y + pt.y - m_dragStart.y;

        HWND parent = GetParent(m_hWindow);
        POINT new_pos = { new_x, new_y };
        ScreenToClient(parent, &new_pos);

        SetWindowPos(m_hWindow, NULL, new_pos.x, new_pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
#else
        int new_x = m_winStart.x + x - m_dragStart.x;
        int new_y = m_winStart.y + y - m_dragStart.y;
        m_target->move(new_x, new_y);
        m_target->updateGeometry();
#endif
    }
}
