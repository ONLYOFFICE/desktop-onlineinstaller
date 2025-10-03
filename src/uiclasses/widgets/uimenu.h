#ifndef UIMENU_H
#define UIMENU_H

#include "uiabstractpopup.h"
#include "uipixmap.h"

class UIButton;
class UIBoxLayout;
class UIOpacityAnimation;
class DECL_VISUALUI UIMenu : public UIAbstractPopup
{
public:
    explicit UIMenu(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_TOOL_RECT);
    virtual ~UIMenu();

    UIButton* addSection(const tstring &text, const UIPixmap &pixmap = UIPixmap());
    void addSeparator();
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
    UIOpacityAnimation *m_animation;
    UIBoxLayout *m_vlut;
};

#endif // UIMENU_H
