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

#include "uiabstractwindow.h"


#ifdef _WIN32
static BOOL CALLBACK ShowChildrenProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    return TRUE;
}
#endif

UIAbstractWindow::UIAbstractWindow(UIWidget *parent, ObjectType type, const Rect &rc) :
    UIWidget(parent, type, nullptr, rc)
{
#ifdef __linux
    gtk_window_set_position(GTK_WINDOW(m_hWindow), GtkWindowPosition::GTK_WIN_POS_CENTER);
    gtk_widget_set_app_paintable(m_hWindow, TRUE);
    GdkScreen *scr = gtk_widget_get_screen(m_hWindow);
    if (GdkVisual *vis = gdk_screen_get_rgba_visual(scr))
        gtk_widget_set_visual(m_hWindow, vis);

    m_gtk_layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(m_hWindow), m_gtk_layout);
#endif
}

UIAbstractWindow::~UIAbstractWindow()
{

}

void UIAbstractWindow::setWindowTitle(const tstring &title)
{
    m_title = title;
#ifdef _WIN32
    SetWindowText(m_hWindow, title.c_str());
#else
    gtk_window_set_title(GTK_WINDOW(m_hWindow), title.c_str());
#endif
}

#ifdef __linux__
void UIAbstractWindow::setGeometry(int x, int y, int w, int h)
{
    gtk_window_resize(GTK_WINDOW(m_hWindow), w, h);
    gtk_window_move(GTK_WINDOW(m_hWindow), x, y);
}

void UIAbstractWindow::move(int x, int y)
{
    gtk_window_move(GTK_WINDOW(m_hWindow), x, y);
}

void UIAbstractWindow::resize(int w, int h)
{
    gtk_window_resize(GTK_WINDOW(m_hWindow), w, h);
}
#endif

Size UIAbstractWindow::size() const
{
#ifdef _WIN32
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    return Size(rc.right - rc.left, rc.bottom - rc.top);
#else
    int w = 0, h = 0;
    gtk_window_get_size(GTK_WINDOW(m_hWindow), &w, &h);
    return Size(w, h);
#endif
}

void UIAbstractWindow::size(int *width, int *height) const
{
#ifdef _WIN32
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    *width = rc.right - rc.left;
    *height =  rc.bottom - rc.top;
#else
    gtk_window_get_size(GTK_WINDOW(m_hWindow), width, height);
#endif
}

Point UIAbstractWindow::pos() const
{
#ifdef _WIN32
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    return Point(rc.left, rc.top);
#else
    int x, y;
    gtk_window_get_position(GTK_WINDOW(m_hWindow), &x, &y);
    return Point(x, y);
#endif
}

void UIAbstractWindow::showAll()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_SHOWNORMAL);
    UpdateWindow(m_hWindow);
    EnumChildWindows(m_hWindow, ShowChildrenProc, 0);
    SetForegroundWindow(m_hWindow);
    SetFocus(m_hWindow);
#else
    gtk_widget_show_all(m_hWindow);
#endif
}

#ifdef _WIN32
bool UIAbstractWindow::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {


    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIAbstractWindow::event(uint ev_type, void *param)
{
    switch (ev_type) {


    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}

GtkWidget* UIAbstractWindow::gtkLayout()
{
    return m_gtk_layout;
}
#endif
