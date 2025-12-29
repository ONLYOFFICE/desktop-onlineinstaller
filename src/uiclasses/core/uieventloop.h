#ifndef UIEVENTLOOP_H
#define UIEVENTLOOP_H

#include "uidefines.h"
#ifdef _WIN32
#else
# include <glib.h>
#endif

class DECL_VISUALUI UIEventLoop
{
public:
    UIEventLoop();
    ~UIEventLoop();

    bool isRunning() const noexcept;
    void exec();
    void exit();

private:
#ifdef _WIN32
    bool m_running;
#else
    GMainLoop *m_loop;
#endif
};

#endif // UIEVENTLOOP_H
