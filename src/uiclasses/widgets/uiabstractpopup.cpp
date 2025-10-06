#include "uiabstractpopup.h"
#include "uidrawningengine.h"
#include "uilayout.h"
#include "uimetrics.h"
#ifdef __linux__
# include "uiutils.h"
#endif


UIAbstractPopup::UIAbstractPopup(UIWidget *parent, const Rect &rc) :
    UIAbstractWindow(parent, ObjectType::PopupType, rc)
{
    int offset = metrics()->value(Metrics::ShadowWidth) * m_dpi_ratio;
#ifdef __linux
    bool usingWayland = UIUtils::windowServer() == UIUtils::WindowServer::WAYLAND;
    gtk_window_set_type_hint(GTK_WINDOW(m_hWindow), usingWayland ? GDK_WINDOW_TYPE_HINT_UTILITY : GDK_WINDOW_TYPE_HINT_POPUP_MENU);
    gtk_window_set_decorated(GTK_WINDOW(m_hWindow), FALSE);
    GdkScreen *scr = gtk_widget_get_screen(m_hWindow);
    if (gdk_screen_is_composited(scr)) {

    } else {
        metrics()->setMetrics(Metrics::ShadowWidth, 0);
        offset = 0;
    }
    gtk_window_set_default_size(GTK_WINDOW(m_hWindow), rc.width + 2 * offset, rc.height + 2 * offset);
    gtk_window_move(GTK_WINDOW(m_hWindow), rc.x - offset, rc.y - offset);
#else
    UIAbstractWindow::setGeometry(rc.x - offset, rc.y - offset, rc.width + 2 * offset, rc.height + 2 * offset);
#endif
}

UIAbstractPopup::~UIAbstractPopup()
{

}

void UIAbstractPopup::setGeometry(int x, int y, int width, int height)
{
    int offset = metrics()->value(Metrics::ShadowWidth) * m_dpi_ratio;
#ifdef _WIN32
    UIAbstractWindow::setGeometry(x - offset, y - offset, width + 2 * offset, height + 2 * offset);
#else
    int _w = width + 2 * offset;
    int _h = height + 2 * offset;
    UIAbstractWindow::setGeometry(x - offset, y - offset, _w, _h);
    if (m_layout)
        m_layout->onResize(_w, _h);
#endif
}

#ifdef __linux__
void UIAbstractPopup::move(int x, int y)
{
    int offset = metrics()->value(Metrics::ShadowWidth);
    UIAbstractWindow::move(x - offset, y - offset);
}

void UIAbstractPopup::resize(int w, int h)
{
    int offset = metrics()->value(Metrics::ShadowWidth);
    int _w = w + 2 * offset;
    int _h = h + 2 * offset;
    UIAbstractWindow::resize(_w, _h);
    if (m_layout)
        m_layout->onResize(_w, _h);
}
#endif

#ifdef _WIN32
bool UIAbstractPopup::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            RECT rc;
            GetClientRect(m_hWindow, &rc);
            if (m_layout)
                m_layout->onResize(rc.right, rc.bottom, m_dpi_ratio);
            paintLayered();
        }
        break;
    }

    case WM_SIZE: {
        UIAbstractWindow::event(msg, wParam, lParam, result);
        paintLayered();
        return false;
    }

    case WM_UPDATE_LAYERED_WINDOW: {
        paintLayered();
        return false;
    }

    default:
        break;
    }
    return UIAbstractWindow::event(msg, wParam, lParam, result);
}

void UIAbstractPopup::onPaintLayered(const RECT &rc, BYTE *opacity)
{

}

void UIAbstractPopup::paintLayered()
{
    UIDrawingEngine *de = engine();
    RECT rc;
    GetClientRect(m_hWindow, &rc);
    de->LayeredBegin(this, m_hWindow, &rc, m_dpi_ratio);
    de->DrawShadow();
    if (metrics()->value(Metrics::BorderRadius) == 0)
        de->DrawFlatRect();
    else
        de->DrawRoundedRect(m_corners, metrics()->value(Metrics::ShadowWidth) * m_dpi_ratio, false);

    BYTE opacity = 255;
    onPaintLayered(rc, &opacity);

    de->LayeredUpdate(opacity);
    de->LayeredEnd();
}
#else
bool UIAbstractPopup::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_HOOKED_DRAW: {
        UIDrawingEngine *de = engine();
        int w = gtk_widget_get_allocated_width(m_hWindow);
        int h = gtk_widget_get_allocated_height(m_hWindow);
        Rect rc(0, 0, w, h);

        de->Begin(this, (cairo_t*)param, &rc);

        de->DrawShadow();
        if (metrics()->value(Metrics::BorderRadius) == 0)
            de->DrawFlatRect();
        else
            de->DrawRoundedRect(m_corners, metrics()->value(Metrics::ShadowWidth) * m_dpi_ratio);

        onPaint(rc);

        de->End();
        return false;
    }

    case GDK_HOOKED_MAP_AFTER:
    case GDK_HOOKED_SIZE_ALLOC: {
        int w = 0, h = 0;
        gtk_window_get_size(GTK_WINDOW(m_hWindow), &w, &h);
        if (m_layout)
            m_layout->onResize(w, h);
        return false;
    }

    default:
        break;
    }
    return UIAbstractWindow::event(ev_type, param);
}

void UIAbstractPopup::onPaint(const RECT&)
{

}
#endif
