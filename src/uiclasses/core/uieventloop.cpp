#include "uieventloop.h"
#ifdef _WIN32
# include <Windows.h>
#else
#endif


UIEventLoop::UIEventLoop()
{
#ifdef _WIN32
    m_running = false;
#else
    m_loop = g_main_loop_new(nullptr, FALSE);
#endif
}

UIEventLoop::~UIEventLoop()
{
#ifdef _WIN32
    if (m_running)
        exit();
#else
    if (m_loop)
        g_main_loop_unref(m_loop);
#endif
}

bool UIEventLoop::isRunning() const noexcept
{
#ifdef _WIN32
    return m_running;
#else
    return (m_loop && g_main_loop_is_running(m_loop));
#endif
}

void UIEventLoop::exec()
{
#ifdef _WIN32
    m_running = true;
    MSG msg;
    BOOL res;
    while (m_running && (res = GetMessage(&msg, NULL, 0, 0)) != 0 && res != -1) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#else
    if (m_loop && !g_main_loop_is_running(m_loop))
        g_main_loop_run(m_loop);
#endif
}

void UIEventLoop::exit()
{
#ifdef _WIN32
    m_running = false;
    PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0); // Wake up GetMessage
#else
    if (m_loop && g_main_loop_is_running(m_loop))
        g_main_loop_quit(m_loop);
#endif
}
