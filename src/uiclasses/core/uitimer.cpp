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
