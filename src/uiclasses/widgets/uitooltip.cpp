#include "uitooltip.h"
#include "uidrawningengine.h"
#include "uiopacityanimation.h"


UIToolTip::UIToolTip(UIWidget *parent, const Rect &rc) :
    UIAbstractPopup(parent, rc),
    m_animation(new UIOpacityAnimation(this))
{
#ifdef _WIN32
    SetProp(m_hWindow, L"BlockActivation", (HANDLE)1);
#else
    gtk_widget_set_can_focus(m_hWindow, FALSE);
    gtk_window_set_accept_focus(GTK_WINDOW(m_hWindow), FALSE);
    gtk_window_set_focus_on_map(GTK_WINDOW(m_hWindow), FALSE);
#endif
}

UIToolTip::~UIToolTip()
{
    delete m_animation; m_animation = nullptr;
}

void UIToolTip::setText(const tstring &text)
{
    m_text = text;
}

void UIToolTip::close()
{
    m_animation->startFadeOut();
}

#ifdef _WIN32
bool UIToolTip::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            m_animation->startFadeIn();
        }
        break;
    }

    case WM_NCHITTEST: {
        *result = HTTRANSPARENT;
        return true;
    }

    case WM_TOPLEVEL_ACTIVATION_NOTIFY:
    case WM_TOPLEVEL_BUTTONDOWN_NOTIFY: {
        close();
        return true;
    }

    case WM_NCDESTROY: {
        RemoveProp(m_hWindow, L"BlockActivation");
        break;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(msg, wParam, lParam, result);
}

void UIToolTip::onPaintLayered(const RECT &rc, BYTE *opacity)
{
    *opacity = m_animation->opacity();
    if (!m_text.empty())
        engine()->DrawString(rc, m_text, m_hFont);
}
#else
bool UIToolTip::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_HOOKED_MAP_AFTER: {
        if (m_animation->isFadingOut()) {
            UIWidget::close();
            return false;
        }
        m_animation->startFadeIn();
        return false;
    }

    case GDK_TOPLEVEL_ACTIVATION_NOTIFY:
    case GDK_TOPLEVEL_BUTTONDOWN_NOTIFY: {
        gtk_widget_hide(m_hWindow); // Prevent input area from being restricted when modal window appears
        UIWidget::close();
        return true;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(ev_type, param);
}

void UIToolTip::onPaint(const RECT &rc)
{
    if (!m_text.empty())
        engine()->DrawString(rc, m_text, m_hFont);

    updateInputRegion(Rect());
}
#endif
