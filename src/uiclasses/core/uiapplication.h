#ifndef UIAPPLICATION_H
#define UIAPPLICATION_H

#include "uidefines.h"
#include "uiobject.h"
#include "uicommon.h"
#ifdef _WIN32
# include <Windows.h>
#else
# include <gtk/gtk.h>
#endif


class UIStyle;
class UIWidget;
class DECL_VISUALUI UIApplication : public UIObject
{
public:
#ifdef _WIN32
    explicit UIApplication(HINSTANCE hInstance, PWSTR cmdline, int cmdshow);
#else
    explicit UIApplication(int argc, char *argv[]);
#endif
    UIApplication(const UIApplication&) = delete;
    ~UIApplication();

    enum LayoutDirection : unsigned char {
        LeftToRight = 0,
        RightToLeft
    };

    UIApplication& operator=(const UIApplication&) = delete;
    static UIApplication *instance();
#ifdef _WIN32
    HINSTANCE moduleHandle();
#else
    static void postEvent(GtkWidget*, uint event_type, void *param);
    static bool sendEvent(GtkWidget*, uint event_type, void *param);
#endif
    void setLayoutDirection(LayoutDirection);
    void setFont(const tstring &font, double pointSize = 10) const;
    LayoutDirection layoutDirection() const;
    tstring font() const;
    UIStyle* style();
    double fontPointSize();

    int exec();
    void exit(int);

private:
    friend class UIWidget;
    UIApplication();

    void registerWidget(UIWidget*, ObjectType, const Rect &rc);

    static UIApplication *inst;
    class UIApplicationPrivate;
    UIApplicationPrivate *d_ptr;
};

#endif // UIAPPLICATION_H
