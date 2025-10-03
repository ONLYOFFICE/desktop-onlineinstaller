#ifndef UITOOLTIPHANDLER_H
#define UITOOLTIPHANDLER_H

#include "uidefines.h"
#include "uicommon.h"

class UIWidget;
class UIToolTip;
class UITimer;
class DECL_VISUALUI UIToolTipHandler
{
public:
    explicit UIToolTipHandler(UIWidget* parent);
    ~UIToolTipHandler();

    void setToolTipText(const tstring &text) noexcept;
    void handleMouseMove();
    void skipToolTip();
    void hideToolTip();

private:
    void onToolTipCheck();

    tstring    m_tooltipText;
    UIWidget  *m_parent;
    UIToolTip *m_tooltip;
    UITimer   *m_checkTimer;
    Point m_lastCursorPos;
    int  m_tooltipTickCounter;
    bool m_tooltipSet;
};

#endif // UITOOLTIPHANDLER_H
