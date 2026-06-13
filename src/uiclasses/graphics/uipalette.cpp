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

#include "uipalette.h"
#ifdef __linux__
# define RGB(r, g ,b) ((DWORD) (((BYTE) (r) | ((WORD) (g) << 8)) | (((DWORD) (BYTE) (b)) << 16)))
#endif


Palette::Palette()
{
    palette[Background][Disabled] = 0x21252b;
    palette[Background][Normal]   = 0x21252b;
    palette[Background][Hover]    = 0x34383f;
    palette[Background][Pressed]  = 0x30343c;
    palette[Border][Disabled]     = 0x21252b;
    palette[Border][Normal]       = 0x21252b;
    palette[Border][Hover]        = 0x34383f;
    palette[Border][Pressed]      = 0x30343c;
    palette[Base][Disabled]       = 0x0000ff;
    palette[Base][Normal]         = 0x0000ff;
    palette[Base][Hover]          = 0x0000ff;
    palette[Base][Pressed]        = 0x0000ff;
    palette[AlternateBase][Disabled] = 0xff0000;
    palette[AlternateBase][Normal]   = 0xff0000;
    palette[AlternateBase][Hover]    = 0xff0000;
    palette[AlternateBase][Pressed]  = 0xff0000;
    palette[Text][Disabled]       = 0xeeeeee;
    palette[Text][Normal]         = 0xeeeeee;
    palette[Text][Hover]          = 0xaaaaaa;
    palette[Text][Pressed]        = 0xaaaaaa;
    palette[Primitive][Disabled]  = 0xeeeeee;
    palette[Primitive][Normal]    = 0xeeeeee;
    palette[Primitive][Hover]     = 0xeeeeee;
    palette[Primitive][Pressed]   = 0xeeeeee;
    palette[AlternatePrimitive][Disabled] = 0x888888;
    palette[AlternatePrimitive][Normal]   = 0x333333;
    palette[AlternatePrimitive][Hover]    = 0x333333;
    palette[AlternatePrimitive][Pressed]  = 0x333333;

    setCurrentState(Normal);
}

Palette::~Palette()
{

}

COLORREF Palette::color(Role role) const noexcept
{
    return RGB((currentColors[role] & 0xff0000) >> 16, (currentColors[role] & 0xff00) >> 8, currentColors[role] & 0xff);
}

void Palette::setColor(Role role, State state, DWORD color) noexcept
{
    palette[role][state] = color;
    currentColors[role] = palette[role][currentState];
}

void Palette::setCurrentState(State state) noexcept
{
    currentColors[Background] = palette[Background][state];
    currentColors[Border]     = palette[Border][state];
    currentColors[Base]       = palette[Base][state];
    currentColors[AlternateBase] = palette[AlternateBase][state];
    currentColors[Text]       = palette[Text][state];
    currentColors[Primitive]  = palette[Primitive][state];
    currentColors[AlternatePrimitive]  = palette[AlternatePrimitive][state];
    currentState = state;
}
