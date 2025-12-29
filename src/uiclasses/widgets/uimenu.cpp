#include "uimenu.h"
#include "uibutton.h"
#include "uiboxlayout.h"
#include "uispacer.h"
#include "uimetrics.h"
#include "uiopacityanimation.h"
#ifdef _WIN32
# include <windowsx.h>
#endif

#define SPACING 2

UIMenu::UIMenu(UIWidget *parent, const Rect &rc) :
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

    m_vlut = new UIBoxLayout(UIBoxLayout::Vertical, UIBoxLayout::AlignCenter);
    m_vlut->setContentMargins(17, 17+2*SPACING, 17, 17+2*SPACING);
    m_vlut->setSpacing(0);
    setLayout(m_vlut);
}

UIMenu::~UIMenu()
{
    delete m_animation; m_animation = nullptr;
}

UIButton* UIMenu::addSection(const tstring &text, const UIPixmap &pixmap)
{
    UIButton *btn = new UIButton(this, text);
    btn->setFont(_T("Arial"), 8.5);
    btn->setObjectGroupId(_T("MenuButton"));
    btn->metrics()->setMetrics(Metrics::IconAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
    btn->metrics()->setMetrics(Metrics::IconMarginLeft, 6);
    btn->setPixmap(pixmap);
    btn->setIconSize(14, 14);
    m_vlut->addWidget(btn);
    return btn;
}

void UIMenu::addSeparator()
{
    m_vlut->addSpacer(new UISpacer(4, 2*SPACING, SizePolicy::Fixed, SizePolicy::Expanding));
    UIWidget *line = new UIWidget(this);
    line->setObjectGroupId(_T("MenuSeparator"));
    m_vlut->addWidget(line);
    m_vlut->addSpacer(new UISpacer(4, 2*SPACING, SizePolicy::Fixed, SizePolicy::Expanding));
}

void UIMenu::close()
{
    m_animation->startFadeOut();
}

#ifdef _WIN32
bool UIMenu::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            m_animation->startFadeIn();
        }
        break;
    }

    case WM_NCHITTEST: {
        RECT rc;
        GetClientRect(m_hWindow, &rc);
        int offset = metrics()->value(Metrics::ShadowWidth) * m_dpi_ratio;
        rc.left += offset;
        rc.top += offset;
        rc.right -= offset;
        rc.bottom -= offset;
        *result = checkInputRegion(lParam, rc);
        return true;
    }

    case WM_TOPLEVEL_ACTIVATION_NOTIFY:
    case WM_TOPLEVEL_BUTTONDOWN_NOTIFY: {
        close();
        return true;
    }

    case WM_TOPLEVEL_KEYDONW_NOTIFY: {
        switch (wParam) {
        case VK_LWIN:
        case VK_RWIN:
        case VK_ESCAPE: {
            close();
            break;
        }
        default:
            break;
        }
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

void UIMenu::onPaintLayered(const RECT &rc, BYTE *opacity)
{
    *opacity = m_animation->opacity();
}
#else
bool UIMenu::event(uint ev_type, void *param)
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

    case GDK_TOPLEVEL_KEYDONW_NOTIFY: {
        GdkEventKey *kev = (GdkEventKey*)param;
        switch (kev->keyval) {
        case GDK_KEY_Super_L:
        case GDK_KEY_Super_R:
        case GDK_KEY_Escape: {
            close();
            break;
        }
        default:
            break;
        }
        return true;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(ev_type, param);
}

void UIMenu::onPaint(const RECT &rc)
{
    int offset = metrics()->value(Metrics::ShadowWidth) * m_dpi_ratio;
    updateInputRegion(Rect(offset, offset, rc.width - 2*offset, rc.height - 2*offset));
}
#endif
