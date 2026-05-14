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

#ifndef UILINEEDIT_H
#define UILINEEDIT_H

#include "uiabstractbutton.h"
#include "uiconhandler.h"


class DECL_VISUALUI UILineEdit : public UIAbstractButton, public UIconHandler
{
public:
    explicit UILineEdit(UIWidget *parent = nullptr, const tstring &text = {});
    ~UILineEdit();

    virtual void setText(const tstring &text) noexcept override;
    void setPlaceholderText(const tstring &text) noexcept;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onPaint(const RECT &rc) override;

private:
    void textBounds(const tstring &text, Rect &rc);

    tstring m_sourceText,
            m_placeholderText,
            m_viewportText;
#ifdef _WIN32
    bool m_caretCreated;
    Gdiplus::RectF m_viewportRc;
#else
    GtkWidget *m_caret;
    Rect m_viewportRc;
#endif
    uint32_t m_pos,
         m_caretPosX,
         m_caretPosY;
};

#endif // UILINEEDIT_H
