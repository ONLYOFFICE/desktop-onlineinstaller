#include "uicheckbox.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#ifdef _WIN32
# include "uipalette.h"
#endif


UICheckBox::UICheckBox(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UICheckBox::~UICheckBox()
{

}

void UICheckBox::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

bool UICheckBox::isChecked()
{
    return m_checked;
}

#ifdef _WIN32
bool UICheckBox::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        return false;
    }

    case WM_LBUTTONUP: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
            click();
        }
        return false;
    }

    case WM_MOUSEENTER: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
        }
        return false;
    }

    case WM_MOUSELEAVE: {
        if (m_mouse_entered) {
            m_mouse_entered = false;
        }
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Normal);
            repaint();
        }
        return false;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UICheckBox::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UICheckBox::onPaint(const RECT&)
{
    engine()->DrawCheckBox(m_text, m_hFont, m_check_rc, m_checked);
#ifdef __linux__
    updateInputRegion(m_check_rc);
#endif
}

void UICheckBox::click()
{
    m_checked = !m_checked;
    update();
    UIAbstractButton::click();
}
