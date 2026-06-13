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

#include "uicaption.h"
#include "uidrawningengine.h"
#ifdef _WIN32
# include "uiutils.h"
# include <windowsx.h>
#endif

#define RESIZE_AREA_PART 0.14


UICaption::UICaption(UIWidget *parent) :
    UILabel(parent),
#ifdef __linux
    m_is_pressed(false),
#endif
    m_isResizingAvailable(true)
{

}

UICaption::~UICaption()
{

}

void UICaption::setResizingAvailable(bool isResizingAvailable)
{
    m_isResizingAvailable = isResizingAvailable;
}

#ifdef _WIN32
bool UICaption::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN: {
        PostMessage(m_root_hWnd, WM_CHILD_BUTTONDOWN_NOTIFY, wParam, lParam);
        if (isResizingAvailable()) {
            int y = GET_Y_LPARAM(lParam);
            if (HCURSOR hCursor = LoadCursor(NULL, isPointInResizeArea(y) ? IDC_SIZENS : IDC_ARROW))
                SetCursor(hCursor);
        }
        if (postMsg(WM_NCLBUTTONDOWN)) {
            *result = TRUE;
            return true;
        }
        return false;
    }

    case WM_LBUTTONDBLCLK: {
        if (postMsg(WM_NCLBUTTONDBLCLK)) {
            *result = TRUE;
            return true;
        }
        return false;
    }

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE: {
        if (isResizingAvailable()) {
            int y = GET_Y_LPARAM(lParam);
            if (HCURSOR hCursor = LoadCursor(NULL, isPointInResizeArea(y) ? IDC_SIZENS : IDC_ARROW))
                SetCursor(hCursor);
        }
        break;
    }

    case WM_MOUSEENTER: {
        //palette()->setCurrentState(Palette::Hover);
        //repaint();
        break;
    }

    case WM_MOUSELEAVE: {
        //palette()->setCurrentState(Palette::Normal);
        //repaint();
        break;
    }

    default:
        break;
    }
    return UILabel::event(msg, wParam, lParam, result);
}
#else
bool UICaption::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_BUTTON_PRESS: {
        UILabel::event(ev_type, param);
        GdkEventButton *bev = (GdkEventButton*)param;
        if (!m_disabled && bev->button == GDK_BUTTON_PRIMARY) {
            m_is_pressed = true;
        }
        return true;
    }

    case GDK_BUTTON_RELEASE: {
        GdkEventButton *bev = (GdkEventButton*)param;
        if (!m_disabled && bev->button == GDK_BUTTON_PRIMARY) {
            m_is_pressed = false;
        }
        return true;
    }

    case GDK_DOUBLE_BUTTON_PRESS: {
        GdkEventButton *bev = (GdkEventButton*)param;
        GtkWidget *root = gtk_widget_get_toplevel(m_hWindow);
        if (!m_disabled && root && bev->button == GDK_BUTTON_PRIMARY) {
            gtk_widget_grab_focus(m_hWindow);
            if (gtk_window_is_maximized(GTK_WINDOW(root))) {
                gtk_window_unmaximize(GTK_WINDOW(root));
            } else {
                gtk_window_maximize(GTK_WINDOW(root));
            }
        }
        return true;
    }

    case GDK_MOTION_NOTIFY: {
        GdkEventMotion *mev = (GdkEventMotion*)param;
        GtkWidget *root = gtk_widget_get_toplevel(m_hWindow);
        if (root && m_is_pressed) {
            m_is_pressed = false;
            gtk_window_begin_move_drag(GTK_WINDOW(root), GDK_BUTTON_PRIMARY, mev->x_root, mev->y_root, mev->time);
            return true;
        }
        return false;
    }

    default:
        break;
    }
    return UILabel::event(ev_type, param);
}
#endif

void UICaption::onPaint(const RECT &rc)
{
    UIDrawingEngine *de = engine();
#ifdef _WIN32
    if (m_hBmp)
        de->DrawImage(m_hBmp);
    if (m_hIcon)
        de->DrawIcon(m_hIcon);
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
}

bool UICaption::isResizingAvailable()
{
#ifdef _WIN32
    return m_isResizingAvailable && UIUtils::winVersion() >= UIUtils::WinVer::Win10 && !IsZoomed(m_root_hWnd);
#else
    return m_isResizingAvailable;
#endif
}

#ifdef _WIN32
bool UICaption::isPointInResizeArea(int posY)
{
    int w = 0, h = 0;
    size(&w, &h);
    return posY <= RESIZE_AREA_PART * h;
}

bool UICaption::postMsg(DWORD cmd) {
    POINT pt;
    ::GetCursorPos(&pt);
    ScreenToClient(m_hWindow, &pt);
    ::ReleaseCapture();
    ::PostMessage(m_root_hWnd, cmd, isResizingAvailable() && isPointInResizeArea(pt.y) ? HTTOP : HTCAPTION, POINTTOPOINTS(pt));
    return true;
}
#endif
