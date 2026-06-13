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

#ifndef UIWINDOW_H
#define UIWINDOW_H

#include "uiabstractwindow.h"

#define DEFAULT_WINDOW_RECT Rect(100,100,1368,768)


class DECL_VISUALUI UIWindow : public UIAbstractWindow
{
public:
    explicit UIWindow(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_WINDOW_RECT, BYTE windowFlags = 0);
    virtual ~UIWindow();

    enum Flags : BYTE {
        RemoveSystemDecoration = 1
    };

    enum State {
#ifdef _WIN32
        Normal = SIZE_RESTORED,
        Minimized = SIZE_MINIMIZED,
        Maximized = SIZE_MAXIMIZED
#else
        Normal = 0,
        Minimized = GDK_WINDOW_STATE_ICONIFIED,
        Maximized = GDK_WINDOW_STATE_MAXIMIZED
#endif
    };

#ifdef _WIN32
    virtual void setGeometry(int, int, int, int) override;
    virtual void move(int, int) override;
#else
    BYTE cornersPlacementAndRadius(int &radius);
    virtual Size size() const override;
    virtual void size(int*, int*) const override;
#endif
    void setMinimumSize(int w, int h);
    void setMaximumSize(int w, int h);
    void setCentralWidget(UIWidget*);
    void setContentsMargins(int, int, int, int);
    void setResizable(bool);
    void showNormal();
    void showMinimized();
    void showMaximized();
#ifdef _WIN32
    void setIcon(int);
#else
    void setIcon(const tstring&);
#endif
    void setLayout(UILayout*) = delete;
    bool isMinimized();
    bool isMaximized();
    UIWidget *centralWidget();
    UILayout *layout() = delete;

    /* callback */
    int onStateChanged(const FnVoidInt &callback);

    virtual void disconnect(int) override;

protected:    
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif

private:
    UIWidget *m_centralWidget;
    Margins  m_contentMargins;
    COLORREF m_brdColor;
    int      m_brdWidth,
             m_resAreaWidth,
             m_state;
    Margins  m_frame;
#ifdef _WIN32
    double   m_dpi;
#else
    GtkWidget *m_client_area;
    int      m_radius;
    bool     m_is_support_round_corners;
#endif
    bool     m_borderless,
             m_isResizable,
             m_isMaximized,
#ifdef _WIN32
             m_isThemeActive,
             m_isTaskbarAutoHideOn,
#endif
             m_scaleChanged;

    Size     m_min_size,
             m_max_size;
#ifdef _WIN32
    Size     m_size;
#endif
    std::unordered_map<int, FnVoidInt> m_state_callbacks;
};

#endif // WINDOW_H
