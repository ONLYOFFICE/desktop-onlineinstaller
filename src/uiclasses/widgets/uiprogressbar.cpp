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

#include "uiprogressbar.h"
#include "uidrawningengine.h"
#include "uitimer.h"
#ifdef __linux__
# include "uiapplication.h"

# define DEFAULT_TIMER_STEP 16
#else
# define DEFAULT_TIMER_STEP USER_TIMER_MINIMUM
#endif
#define DEFAULT_PULSE_STEP 1

UIProgressBar::UIProgressBar(UIWidget *parent) :
    UIWidget(parent, ObjectType::WidgetType),
    m_timer(new UITimer),
    m_progress(0),
    m_pulse_pos(-1),
    m_pulse_direction(1),
    m_pulse_step(DEFAULT_PULSE_STEP)
{
    m_timer->onTimeout([this]() {
        m_pulse_pos += m_pulse_direction * m_pulse_step;
        if (m_pulse_pos >= 100) {
            m_pulse_pos = 100;
            m_pulse_direction = -1;
        } else if (m_pulse_pos <= 0) {
            m_pulse_pos = 0;
            m_pulse_direction = 1;
        }
        update();
    });
}

UIProgressBar::~UIProgressBar()
{
    delete m_timer; m_timer = nullptr;
}

void UIProgressBar::setProgress(int progress)
{
    m_progress = progress;
    update();
}

void UIProgressBar::pulse(bool enable)
{
    m_pulse_pos = enable ? 0 : -1;
    m_pulse_direction = 1;
    if (enable) {
        m_timer->start(DEFAULT_TIMER_STEP);
    } else {
        m_timer->stop();
    }
}

void UIProgressBar::setPulseStep(int step)
{
    if (step < 1)
        step = 1;
    else
    if (step > 50)
        step = 50;
    m_pulse_step = step;
}

#ifdef _WIN32
bool UIProgressBar::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIProgressBar::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}
#endif

void UIProgressBar::onPaint(const RECT&)
{
    engine()->DrawProgressBar(m_progress, m_pulse_pos);
}
