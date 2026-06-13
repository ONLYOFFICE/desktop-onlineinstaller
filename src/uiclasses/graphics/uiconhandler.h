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

#ifndef UICONHANDLER_H
#define UICONHANDLER_H

#include "uidefines.h"
#include "uipixmap.h"
#ifdef _WIN32
# define setVectorIcon(id, w, h) setEMFIcon(id, w, h)
#else
# define setVectorIcon(id, w, h) setSVGIcon(id, w, h)
#endif

class UIWidget;
struct _RsvgHandle;
class DECL_VISUALUI UIconHandler
{
public:
    explicit UIconHandler(UIWidget *owner);
    virtual ~UIconHandler();

    void setIcon(const tstring &path, int w, int h);
#ifdef _WIN32
    void setIcon(int id, int w, int h);
    void setEMFIcon(const tstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setImage(int id, int w, int h);
#else
    void setIcon(const char *id, int w, int h);
    void setSVGIcon(const tstring &path, int w, int h);
    void setSVGIcon(const char *id, int w, int h);
    void setImage(const char *id, int w, int h);
#endif
    void setImage(const tstring &path, int w, int h);
    void setPixmap(const UIPixmap &pixmap);
    void setPixmap(UIPixmap &&pixmap);
    void setIconSize(int w, int h);

protected:
    PlatformBitmap *m_hBmp;
#ifdef _WIN32
    HICON m_hIcon;
    Gdiplus::Metafile *m_hEmf;
#else
    _RsvgHandle *m_hSvg;
#endif

private:
    UIWidget *m_owner;
};

#endif // UICONHANDLER_H
