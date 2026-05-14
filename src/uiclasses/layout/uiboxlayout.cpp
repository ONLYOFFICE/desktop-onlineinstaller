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

#include "uiboxlayout.h"
#include "uiwidget.h"
#include "uispacer.h"
#include <cmath>


UIBoxLayout::UIBoxLayout(Direction direction, int alignment) :
    UILayout(alignment),
    m_direction(direction)
{
    m_margins = Margins(6,6,6,6);
    m_spacing = 6;
}

UIBoxLayout::~UIBoxLayout()
{
    for (auto it = m_destroy_conn.begin(); it != m_destroy_conn.end(); it++)
        it->first->disconnect(it->second);

    for (const UILayoutItem &item : m_items) {
        if (item.spr)
            delete item.spr;
    }
}

void UIBoxLayout::addWidget(UIWidget *wgt)
{
    UILayoutItem item;
    item.wgt = wgt;
    item.hsb = wgt->sizePolicy(SizePolicy::HSizeBehavior);
    item.vsb = wgt->sizePolicy(SizePolicy::VSizeBehavior);
    m_items.push_back(item);
    // int destroy_conn = wgt->onAboutToDestroy([=]() {
    //     auto it = std::find(m_widgets.begin(), m_widgets.end(), wgt);
    //     if (it != m_widgets.end())
    //         m_widgets.erase(it);

    //     auto it_conn = m_destroy_conn.find(wgt);
    //     if (it_conn != m_destroy_conn.end())
    //         m_destroy_conn.erase(it_conn);
    // });
    // m_destroy_conn[wgt] = destroy_conn;
}

void UIBoxLayout::addSpacer(UISpacer *spr)
{
    UILayoutItem item;
    item.spr = spr;
    item.hsb = spr->sizePolicy(SizePolicy::HSizeBehavior);
    item.vsb = spr->sizePolicy(SizePolicy::VSizeBehavior);
    m_items.push_back(item);
}

void UIBoxLayout::setContentMargins(int left, int top, int right, int bottom) noexcept
{
    m_margins = Margins(left, top, right, bottom);
}

void UIBoxLayout::setSpacing(int spacing) noexcept
{
    m_spacing = spacing;
}

void UIBoxLayout::onResize(int w, int h, double dpi) noexcept
{
    m_width = w; m_height = h;
    int amount = m_items.size();
    if (amount > 0) {
        int x = m_margins.left * dpi;
        int y = m_margins.top * dpi;
        int sum_width = w - (m_margins.right + m_margins.left) * dpi;
        int sum_height = h - (m_margins.bottom + m_margins.top) * dpi;
        int spacing = m_spacing * dpi;
        int num_fixed = 0;
        int sum_fixed_width_or_height = 0;
        int last_expanding = -1;
        for (int i = 0; i < amount; ++i) {
            UILayoutItem &item = m_items[i];
            // item.hsb = item.wgt->sizePolicy(Widget::HSizeBehavior);
            // item.vsb = item.wgt->sizePolicy(Widget::VSizeBehavior);
            item.calcSize(dpi);
            if (item.hsb == SizePolicy::Fixed && m_direction == Horizontal) {
                sum_fixed_width_or_height += item.width;
                ++num_fixed;
            } else
            if (item.vsb == SizePolicy::Fixed && m_direction == Vertical) {
                sum_fixed_width_or_height += item.height;
                ++num_fixed;
            } else
            if ((m_direction == Horizontal && item.hsb == SizePolicy::Expanding) ||
                    (m_direction == Vertical && item.vsb == SizePolicy::Expanding)) {
                last_expanding = i;
            }
        }

        if (m_direction == Horizontal) {
            sum_width -= (amount - 1) * spacing;

            if (num_fixed != 0 && last_expanding != -1) {
                int sep_width = (int)std::round((float)(sum_width - sum_fixed_width_or_height)/(amount - num_fixed));
                for (int i = 0; i < amount; i++) {
                    if (i == last_expanding)
                        sep_width = (sum_width - sum_fixed_width_or_height) - (amount - num_fixed - 1)*sep_width;
                    UILayoutItem &item = m_items[i];
                    if (item.hsb == SizePolicy::Fixed) {
                        if (item.vsb == SizePolicy::Fixed) {
                            int _y = y;
                            _y += (m_alignment & AlignVCenter) ? (int)std::round((float)(sum_height - item.height) / 2) : (m_alignment & AlignVBottom) ? sum_height - item.height : 0;
                            item.move(x, _y);
                        } else
                        if (item.vsb == SizePolicy::Expanding) {
                            item.setGeometry(x, y, item.width, sum_height);
                        }
                        x += item.width + spacing;

                    } else
                    if (item.hsb == SizePolicy::Expanding) {
                        if (item.vsb == SizePolicy::Fixed) {
                            int _y = y;
                            _y += (m_alignment & AlignVCenter) ? (int)std::round((float)(sum_height - item.height) / 2) : (m_alignment & AlignVBottom) ? sum_height - item.height : 0;
                            item.setGeometry(x, _y, sep_width, item.height);
                        } else
                        if (item.vsb == SizePolicy::Expanding) {
                            item.setGeometry(x, y, sep_width, sum_height);
                        }
                        x += sep_width + spacing;
                    }
                }

            } else {
                int sep_width = (int)std::round((float)sum_width/amount);
                for (int i = 0; i < amount; i++) {
                    if (i == amount - 1)
                        sep_width = sum_width - i*sep_width;
                    UILayoutItem &item = m_items[i];
                    if (item.hsb == SizePolicy::Fixed) {
                        int _x = x;
                        _x += (m_alignment & AlignHCenter) ? (int)std::round((float)(sep_width - item.width) / 2) : (m_alignment & AlignHRight) ? sep_width - item.width : 0;
                        if (item.vsb == SizePolicy::Fixed) {
                            int _y = y;
                            _y += (m_alignment & AlignVCenter) ? (int)std::round((float)(sum_height - item.height) / 2) : (m_alignment & AlignVBottom) ? sum_height - item.height : 0;
                            item.move(_x, _y);
                        } else
                        if (item.vsb == SizePolicy::Expanding) {
                            item.setGeometry(_x, y, item.width, sum_height);
                        }

                    } else
                    if (item.hsb == SizePolicy::Expanding) {
                        if (item.vsb == SizePolicy::Fixed) {
                            int _y = y;
                            _y += (m_alignment & AlignVCenter) ? (int)std::round((float)(sum_height - item.height) / 2) : (m_alignment & AlignVBottom) ? sum_height - item.height : 0;
                            item.setGeometry(x, _y, sep_width, item.height);
                        } else
                        if (item.vsb == SizePolicy::Expanding) {
                            item.setGeometry(x, y, sep_width, sum_height);
                        }
                    }
                    x += sep_width + spacing;
                }
            }

        } else {
            sum_height -= (amount - 1) * spacing;

            if (num_fixed != 0 && last_expanding != -1) {
                int sep_height = (int)std::round((float)(sum_height - sum_fixed_width_or_height)/(amount - num_fixed));
                for (int i = 0; i < amount; i++) {
                    if (i == last_expanding)
                        sep_height = (sum_height - sum_fixed_width_or_height) - (amount - num_fixed - 1)*sep_height;
                    UILayoutItem &item = m_items[i];
                    if (item.vsb == SizePolicy::Fixed) {
                        if (item.hsb == SizePolicy::Fixed) {
                            int _x = x;
                            _x += (m_alignment & AlignHCenter) ? (int)std::round((float)(sum_width - item.width) / 2) : (m_alignment & AlignHRight) ? sum_width - item.width : 0;
                            item.move(_x, y);
                        } else
                        if (item.hsb == SizePolicy::Expanding) {
                            item.setGeometry(x, y, sum_width, item.height);
                        }
                        y += item.height + spacing;

                    } else
                    if (item.vsb == SizePolicy::Expanding) {
                        if (item.hsb == SizePolicy::Fixed) {
                            int _x = x;
                            _x += (m_alignment & AlignHCenter) ? (int)std::round((float)(sum_width - item.width) / 2) : (m_alignment & AlignHRight) ? sum_width - item.width : 0;
                            item.setGeometry(_x, y, item.width, sep_height);
                        } else
                        if (item.hsb == SizePolicy::Expanding) {
                            item.setGeometry(x, y, sum_width, sep_height);
                        }
                        y += sep_height + spacing;
                    }
                }

            } else {
                int sep_height = (int)std::round((float)sum_height/amount);
                for (int i = 0; i < amount; i++) {
                    if (i == amount - 1)
                        sep_height = sum_height - i*sep_height;
                    UILayoutItem &item = m_items[i];
                    if (item.vsb == SizePolicy::Fixed) {
                        int _y = y;
                        _y += (m_alignment & AlignVCenter) ? (int)std::round((float)(sep_height - item.height) / 2) : (m_alignment & AlignVBottom) ? sep_height - item.height : 0;
                        if (item.hsb == SizePolicy::Fixed) {
                            int _x = x;
                            _x += (m_alignment & AlignHCenter) ? (int)std::round((float)(sum_width - item.width) / 2) : (m_alignment & AlignHRight) ? sum_width - item.width : 0;
                            item.move(_x, _y);
                        } else
                        if (item.hsb == SizePolicy::Expanding) {
                            item.setGeometry(x, _y, sum_width, item.height);
                        }

                    } else
                    if (item.vsb == SizePolicy::Expanding) {
                        if (item.hsb == SizePolicy::Fixed) {
                            int _x = x;
                            _x += (m_alignment & AlignHCenter) ? (int)std::round((float)(sum_width - item.width) / 2) : (m_alignment & AlignHRight) ? sum_width - item.width : 0;
                            item.setGeometry(_x, y, item.width, sep_height);
                        } else
                        if (item.hsb == SizePolicy::Expanding) {
                            item.setGeometry(x, y, sum_width, sep_height);
                        }
                    }
                    y += sep_height + spacing;
                }
            }
        }
    }
}
