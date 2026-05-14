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

#ifndef UICOMMON_H
#define UICOMMON_H

#include "uidefines.h"
#ifdef __linux__
# include "gtk/gtk.h"
#endif

struct DECL_VISUALUI Margins {
    Margins();
    Margins(int, int, int, int);

    int left, top, right, bottom;
};

struct DECL_VISUALUI Rect {
    Rect();
    Rect(int, int, int, int);

    int x, y, width, height;
};

struct DECL_VISUALUI Point {
    Point();
    Point(int, int);

    int x, y;
};

struct DECL_VISUALUI Size {
    Size();
    Size(int, int);

    int width, height;
};

#ifdef __linux__
typedef Point POINT;
typedef Rect RECT;

struct DECL_VISUALUI DropFilesInfo {
    GdkDragContext *context;
    gint x, y;
    GtkSelectionData *sel_data;
    guint info, time;
};
#endif

struct DECL_VISUALUI FontInfo {
    FontInfo(std::string _name = DEFAULT_FONT_NAME, float _pointSize = 10.0f, int _weight = 400,
             bool _italic = false, bool _underline = false, bool _strikeOut = false) :
        name(_name),
        pointSize(_pointSize),
        weight(_weight),
        italic(_italic),
        underline(_underline),
        strikeOut(_strikeOut)
    {}

    std::string name;
    float pointSize;
    int weight;
    bool italic;
    bool underline;
    bool strikeOut;
};

namespace SizePolicy
{
    enum Properties : unsigned char {
        HSizeBehavior,
        VSizeBehavior,
        PROPERTIES_LAST
    };

    enum SizeBehavior : unsigned char {
        Fixed,
        Expanding,
        SIZE_BEHAVIOR_LAST
    };
}

#endif // COMMON_H
