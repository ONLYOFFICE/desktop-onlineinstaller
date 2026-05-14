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

#ifndef UILAYOUT_H
#define UILAYOUT_H

#include "uidefines.h"
#include "uilayoutitem.h"
#include "uicommon.h"


class UIWidget;
class DECL_VISUALUI UILayout
{
public:
    enum Alignment : unsigned char {
        AlignHLeft   = 1,
        AlignHCenter = 2,
        AlignHRight  = 4,
        AlignVTop    = 8,
        AlignVCenter = 16,
        AlignVBottom = 32,
        AlignCenter  = AlignHCenter | AlignVCenter
    };

    explicit UILayout(int alignment = AlignHLeft | AlignVTop);
    virtual ~UILayout();

    virtual void addWidget(UIWidget *wgt) = 0;
    virtual void addSpacer(UISpacer *spr) = 0;

protected:
    std::vector<UILayoutItem> m_items;
    Margins m_margins;
    int m_alignment,
        m_spacing,
        m_width,
        m_height;

private:
    friend class UIWidget;
    friend class UIDialog;
    friend class UIAbstractPopup;
    virtual void onResize(int w, int h, double dpi = 1.0) noexcept = 0;
};

#endif // UILAYOUT_H
