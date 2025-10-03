#ifndef UIDEFINES_H
#define UIDEFINES_H

#include <functional>
#include <vector>
#include <string>

#if defined(_WIN32)
# define DECL_EXPORT __declspec(dllexport)
# define DECL_IMPORT __declspec(dllimport)
#else
# define DECL_EXPORT __attribute__((visibility("default")))
# define DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(VISUALUI_LIBRARY)
# define DECL_VISUALUI DECL_EXPORT
#elif defined(VISUALUI_STATIC)
# define DECL_VISUALUI
#else
# define DECL_VISUALUI DECL_IMPORT
#endif

#ifdef _WIN32
# include <tchar.h>
  typedef std::wstring tstring;
#else
# define _T(x) x
  typedef std::string tstring;
#endif

#ifdef _WIN32
# define SNAP_LAYOUTS_TIMER_ID    0x1f000000

# define WM_MOUSEENTER            (WM_APP + 1)
# define WM_INVOKEMETHOD          (WM_APP + 2)
# define WM_PARENT_ACTIVATION_NOTIFY   (WM_APP + 3)
# define WM_TOPLEVEL_ACTIVATION_NOTIFY (WM_APP + 4)
# define WM_CHILD_BUTTONDOWN_NOTIFY    (WM_APP + 5)
# define WM_TOPLEVEL_BUTTONDOWN_NOTIFY (WM_APP + 6)
# define WM_CHILD_KEYDOWN_NOTIFY       (WM_APP + 7)
# define WM_TOPLEVEL_KEYDONW_NOTIFY    (WM_APP + 8)
# define WM_SETTINGCHANGE_NOTIFY  (WM_APP + 9)
# define WM_DPICHANGED_NOTIFY     (WM_APP + 10)
# define WM_PAINT_LAYERED_CHILD   (WM_APP + 11)
# define WM_UPDATE_LAYERED_WINDOW (WM_APP + 12)
#else
# include <gdk/gdk.h>
# define GDK_INVOKEMETHOD         (GDK_EVENT_LAST + 1)
# define GDK_PARENT_ACTIVATION_NOTIFY   (GDK_EVENT_LAST + 2)
# define GDK_TOPLEVEL_ACTIVATION_NOTIFY (GDK_EVENT_LAST + 3)
# define GDK_CHILD_BUTTONDOWN_NOTIFY    (GDK_EVENT_LAST + 4)
# define GDK_TOPLEVEL_BUTTONDOWN_NOTIFY (GDK_EVENT_LAST + 5)
# define GDK_TOPLEVEL_KEYDONW_NOTIFY    (GDK_EVENT_LAST + 6)
# define GDK_HOOKED_DRAW          (GDK_EVENT_LAST + 7)
# define GDK_HOOKED_DESTROY       (GDK_EVENT_LAST + 8)
# define GDK_HOOKED_CONFIGURE_AFTER    (GDK_EVENT_LAST + 9)
# define GDK_HOOKED_FOCUS_CHANGE_AFTER (GDK_EVENT_LAST + 10)
# define GDK_HOOKED_WINDOW_STATE_AFTER (GDK_EVENT_LAST + 11)
# define GDK_HOOKED_SIZE_ALLOC         (GDK_EVENT_LAST + 12)
# define GDK_HOOKED_CONFIGURE     (GDK_EVENT_LAST + 13)
# define GDK_HOOKED_REALIZE       (GDK_EVENT_LAST + 14)
# define GDK_HOOKED_BUTTON_PRESS_AFTER (GDK_EVENT_LAST + 15)
# define GDK_HOOKED_DBLBUTTON_PRESS_AFTER (GDK_EVENT_LAST + 16)
# define GDK_HOOKED_MAP_AFTER          (GDK_EVENT_LAST + 17)
# define GDK_HOOKED_DROPFILES          (GDK_EVENT_LAST + 18)
#endif

typedef std::function<void(void)>     FnVoidVoid;
typedef std::function<void(bool)>     FnVoidBool;
typedef std::function<void(bool*)>    FnVoidBoolPtr;
typedef std::function<void(int)>      FnVoidInt;
typedef std::function<void(int,int)>  FnVoidIntInt;
typedef std::function<void(std::vector<tstring>)>  FnVoidVecStr;

#endif // UIDEFINES_H
