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

#include "uitimer.h"

#ifdef _WIN32
std::unordered_map<UINT_PTR, UITimer*> UITimer::m_timer_map = {};
#endif

UITimer::UITimer() :
    m_timerId(0),
    m_currentMode(Mode::Repeating)
{

}

UITimer::~UITimer()
{
    stop();
}

void UITimer::start(int intervalMs, Mode mode)
{
    stop();
    m_currentMode = mode;
#ifdef _WIN32
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    timeBeginPeriod(tc.wPeriodMin);

    m_timerId = SetTimer(NULL, 0, intervalMs, Timerproc);
    m_timer_map[m_timerId] = this;
#else
    m_timerId = g_timeout_add_full(G_PRIORITY_DEFAULT, intervalMs, TimerProc, this, nullptr);
#endif
}

void UITimer::stop()
{
    if (m_timerId != 0) {
#ifdef _WIN32
        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        timeEndPeriod(tc.wPeriodMin);

        KillTimer(NULL, m_timerId);
        auto it = m_timer_map.find(m_timerId);
        if (it != m_timer_map.end())
            m_timer_map.erase(it);
#else
        g_source_remove(m_timerId);    
#endif
        m_timerId = 0;
    }
}

bool UITimer::isActive() const
{
    return m_timerId != 0;
}

void UITimer::onTimeout(FnVoidVoid callback)
{
    m_timeout_callback = callback;
}

void UITimer::handleTimerTick()
{
    if (m_timeout_callback)
        m_timeout_callback();
    if (m_currentMode == Mode::SingleShot)
        stop();
}

#ifdef _WIN32
VOID CALLBACK UITimer::Timerproc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    auto it = m_timer_map.find(idEvent);
    if (it != m_timer_map.end()) {
        if (UITimer* timer = it->second) {
            timer->handleTimerTick();
        }
    }
}
#else
gboolean UITimer::TimerProc(gpointer data)
{
    if (UITimer* timer = (UITimer*)data) {
        timer->handleTimerTick();
    }
    return G_SOURCE_CONTINUE;
}
#endif
