#ifndef UITHREAD_H
#define UITHREAD_H

#include "uiwidget.h"
#ifdef __linux__
# include "uiapplication.h"
#endif


namespace UIThread
{
    template<typename Widget, typename Fn, typename... Args>
    typename std::enable_if<std::is_member_function_pointer<Fn>::value>::type
    invoke(Widget *wgt, Fn&& fn, Args&&... args) {
        auto func = new std::function<void()>(std::bind(std::forward<Fn>(fn), wgt, std::forward<Args>(args)...));
#ifdef _WIN32
        PostMessage(wgt->platformWindow(), WM_INVOKEMETHOD, (WPARAM)func, 0);
#else
        UIApplication::postEvent(wgt->platformWindow(), GDK_INVOKEMETHOD, (void*)func);
#endif
    }   // NOLINT

    template<typename Widget, typename Fn>
    typename std::enable_if<!std::is_member_function_pointer<Fn>::value>::type
    invoke(Widget *wgt, Fn&& fn) {
        auto func = new std::function<void()>(std::forward<Fn>(fn));
#ifdef _WIN32
        PostMessage(wgt->platformWindow(), WM_INVOKEMETHOD, (WPARAM)func, 0);
#else
        UIApplication::postEvent(wgt->platformWindow(), GDK_INVOKEMETHOD, (void*)func);
#endif
    }   // NOLINT
};

#endif // UITHREAD_H
