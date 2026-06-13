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

#ifndef UIMENU_H
#define UIMENU_H

#include "uiabstractpopup.h"
#include "uipixmap.h"

class UIButton;
class UIBoxLayout;
class UIOpacityAnimation;
class DECL_VISUALUI UIMenu : public UIAbstractPopup
{
public:
    explicit UIMenu(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_TOOL_RECT);
    virtual ~UIMenu();

    UIButton* addSection(const tstring &text, const UIPixmap &pixmap = UIPixmap());
    void addSeparator();
    virtual void close() override;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void onPaintLayered(const RECT &rc, BYTE *opacity) override;
#else
    virtual bool event(uint ev_type, void *param) override;
    virtual void onPaint(const RECT &rc) override;
#endif

private:
    UIOpacityAnimation *m_animation;
    UIBoxLayout *m_vlut;
};

#endif // UIMENU_H
