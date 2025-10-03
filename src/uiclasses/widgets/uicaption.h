#ifndef UICAPTION_H
#define UICAPTION_H

#include "uilabel.h"
#ifdef _WIN32
# include <Windows.h>
#endif


class DECL_VISUALUI UICaption : public UILabel
{
public:
    explicit UICaption(UIWidget *parent = nullptr);
    ~UICaption();

    void setResizingAvailable(bool);

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onPaint(const RECT &rc) override;

private:
    bool isResizingAvailable();
#ifdef _WIN32
    bool isPointInResizeArea(int posY);
    bool postMsg(DWORD cmd);
#else
    bool m_is_pressed;
#endif
    bool m_isResizingAvailable;
};

#endif // UICAPTION_H
