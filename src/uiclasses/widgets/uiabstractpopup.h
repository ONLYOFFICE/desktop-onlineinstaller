#ifndef UIABSTRACTPOPUP_H
#define UIABSTRACTPOPUP_H

#include "uiabstractwindow.h"

#define DEFAULT_TOOL_RECT Rect(100,100,800,600)


class DECL_VISUALUI UIAbstractPopup : public UIAbstractWindow
{
public:
    explicit UIAbstractPopup(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_TOOL_RECT);
    virtual ~UIAbstractPopup();

    virtual void setGeometry(int, int, int, int) override;
#ifdef __linux__
    virtual void move(int, int) override;
    virtual void resize(int, int) override;
#endif
    void setWindowTitle(const tstring &title) = delete;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void onPaintLayered(const RECT &rc, BYTE *opacity);
#else
    virtual bool event(uint ev_type, void *param) override;
    virtual void onPaint(const RECT &rc) override;
#endif

private:
#ifdef _WIN32
    void paintLayered();
#endif
};

#endif // UIABSTRACTPOPUP_H
