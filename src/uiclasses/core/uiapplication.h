#ifndef APPLICATION_H
#define APPLICATION_H

#include "uiobject.h"
#include "uicommon.h"
#include <Windows.h>


class UIWidget;
class UIApplication : public UIObject
{
public:
    UIApplication(HINSTANCE hInstance, PWSTR cmdline, int cmdshow);
    UIApplication(const UIApplication&) = delete;
    ~UIApplication();

    enum LayoutDirection : unsigned char {
        LeftToRight = 0,
        RightToLeft
    };

    UIApplication& operator=(const UIApplication&) = delete;
    static UIApplication *instance();
    HINSTANCE moduleHandle();
    void setLayoutDirection(LayoutDirection);
    void setFont(const std::wstring &font) const;
    std::wstring font() const;

    int exec();
    void exit(int);

private:
    UIApplication();
    friend class UIWidget;
    void registerWidget(UIWidget*, ObjectType, const Rect &rc);
    class ApplicationPrivate;
    ApplicationPrivate *d_ptr;
    static UIApplication *inst;
};

#endif // APPLICATION_H
