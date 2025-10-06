#include "uidialog.h"
#include "uilayout.h"
#include "uidrawningengine.h"
#include "uieventloop.h"


UIDialog::UIDialog(UIWidget *parent, const Rect &rc) :
    UIAbstractWindow(parent, ObjectType::DialogType, rc),
    m_loop(new UIEventLoop),
    m_result(DialogCode::Rejected)
{
#ifdef __linux
    gtk_window_set_type_hint(GTK_WINDOW(m_hWindow), GDK_WINDOW_TYPE_HINT_MENU);
    gtk_window_set_title(GTK_WINDOW(m_hWindow), "");
    gtk_window_set_resizable(GTK_WINDOW(m_hWindow), FALSE);
    gtk_window_set_modal(GTK_WINDOW(m_hWindow), TRUE);

    GdkPixbuf *empty_icon = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 1, 1);
    gdk_pixbuf_fill(empty_icon, 0x00000000);
    gtk_window_set_icon(GTK_WINDOW(m_hWindow), empty_icon);
    g_object_unref(empty_icon);

    gtk_window_set_default_size(GTK_WINDOW(m_hWindow), rc.width, rc.height);
    gtk_widget_realize(m_hWindow);
    gtk_window_move(GTK_WINDOW(m_hWindow), rc.x, rc.y);
#endif
}

UIDialog::~UIDialog()
{
    delete m_loop; m_loop = nullptr;
}

#ifdef __linux__
void UIDialog::setGeometry(int x, int y, int w, int h)
{
    UIAbstractWindow::setGeometry(x, y, w, h);
    if (m_layout)
        m_layout->onResize(w, h);
}

void UIDialog::resize(int w, int h)
{
    UIAbstractWindow::resize(w, h);
    if (m_layout)
        m_layout->onResize(w, h);
}
#endif

void UIDialog::accept() noexcept
{
    m_result = DialogCode::Accepted;
    m_loop->exit();
}

void UIDialog::reject() noexcept
{
    m_result = DialogCode::Rejected;
    m_loop->exit();
}

int UIDialog::runDialog()
{
    showAll();
    m_loop->exec();
#ifdef _WIN32
    if (UIWidget *parent = parentWidget())
        EnableWindow(parent->platformWindow(), TRUE);
#endif
    hide();
    return m_result;
}

#ifdef _WIN32
bool UIDialog::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            if (UIWidget *parent = parentWidget())
                EnableWindow(parent->platformWindow(), FALSE);
        }
        break;
    }

    case WM_CLOSE: {
        if (UIWidget *parent = parentWidget())
            EnableWindow(parent->platformWindow(), TRUE);
        if (m_loop->isRunning()) {
            reject();
            *result = TRUE;
            return true;
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractWindow::event(msg, wParam, lParam, result);
}
#else
bool UIDialog::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_HOOKED_DRAW: {
        UIDrawingEngine *de = engine();
        int x = 0, y = 0, w = 0, h = 0;
        w = gtk_widget_get_allocated_width(m_gtk_layout);
        h = gtk_widget_get_allocated_height(m_gtk_layout);
        gtk_widget_translate_coordinates(m_gtk_layout, m_hWindow, 0, 0, &x, &y);
        Rect rc(x, y, w, h);
        de->Begin(this, (cairo_t*)param, &rc);
        de->DrawRoundedRect(m_corners);
        de->End();
        return false;
    }

    case GDK_HOOKED_MAP_AFTER:
        gtk_widget_queue_resize(m_hWindow); // gtk_widget_queue_allocate(m_hWindow);
    case GDK_HOOKED_SIZE_ALLOC: {
        int w = 0, h = 0;
        gtk_window_get_size(GTK_WINDOW(m_hWindow), &w, &h);
        if (m_layout)
            m_layout->onResize(w, h);
        return false;
    }

    case GDK_BUTTON_PRESS: {
        return false;
    }

    case GDK_DELETE: {
        if (m_loop->isRunning()) {
            reject();
            return true;
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractWindow::event(ev_type, param);
}
#endif
