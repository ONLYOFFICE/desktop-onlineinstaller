#include "uitogglebutton.h"
#include "uidrawningengine.h"
#include "uimetrics.h"


UIToggleButton::UIToggleButton(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UIToggleButton::~UIToggleButton()
{

}

void UIToggleButton::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

bool UIToggleButton::isChecked()
{
    return m_checked;
}

void UIToggleButton::onPaint(const RECT&)
{
    engine()->DrawToggleButton(m_text, m_hFont, m_check_rc, m_checked);
#ifdef __linux__
    updateInputRegion(m_check_rc);
#endif
}

void UIToggleButton::click()
{
    m_checked = !m_checked;
    update();
    UIAbstractButton::click();
}
