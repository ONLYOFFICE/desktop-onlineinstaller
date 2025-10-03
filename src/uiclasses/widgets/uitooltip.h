#ifndef UITOOLTIP_H
#define UITOOLTIP_H

#include "uiabstractpopup.h"

class UIOpacityAnimation;
class DECL_VISUALUI UIToolTip : public UIAbstractPopup
{
public:
    explicit UIToolTip(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_TOOL_RECT);
    ~UIToolTip();

    void setText(const tstring &text);
    virtual void close() override;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void onPaintLayered(const RECT &rc, BYTE *opacity) override;
#else
    virtual bool event(uint ev_type, void *param) override;
    virtual void onPaint(const RECT &rc) override;
#endif

private:
    tstring m_text;
    UIOpacityAnimation *m_animation;
};

#endif // UITOOLTIP_H
