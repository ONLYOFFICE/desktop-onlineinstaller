#include "uiradiobutton.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include <windowsx.h>


UIRadioButton::UIRadioButton(UIWidget *parent, const std::wstring &text) :
    UIAbstractButton(parent, text),
    m_checked(false)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UIRadioButton::~UIRadioButton()
{

}

void UIRadioButton::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

bool UIRadioButton::isChecked()
{
    return m_checked;
}

bool UIRadioButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->DrawRadioButton(m_text, m_hFont, m_checked);
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();

        engine()->End();

        *result = FALSE;
        return true;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}

void UIRadioButton::click()
{
    m_checked = true;
    update();
    UIAbstractButton::click();
}
