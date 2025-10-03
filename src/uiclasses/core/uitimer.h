#ifndef UITIMER_H
#define UITIMER_H

#include "uidefines.h"
#ifdef _WIN32
# include <Windows.h>
# include <unordered_map>
#else
# include <glib.h>
#endif

class DECL_VISUALUI UITimer
{
public:
    enum class Mode {
        SingleShot,
        Repeating
    };

    UITimer();
    ~UITimer();

    void start(int intervalMs, Mode mode = Mode::Repeating);
    void stop();
    bool isActive() const;

    /* callback */
    void onTimeout(FnVoidVoid callback);

private:   
    void handleTimerTick();
#ifdef _WIN32
    static VOID CALLBACK Timerproc(HWND, UINT, UINT_PTR, DWORD);
    static std::unordered_map<UINT_PTR, UITimer*> m_timer_map;
    UINT_PTR m_timerId;
#else
    static gboolean TimerProc(gpointer);
    guint m_timerId;
#endif
    FnVoidVoid m_timeout_callback;
    Mode m_currentMode;
};

#endif // UITIMER_H
