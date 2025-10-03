#include "uiwidget.h"
#include "uiapplication.h"
#include "uilayout.h"
#include "uimetrics.h"
#include "uipalette.h"
#include "uidrawningengine.h"
#include "uidraghandler.h"
#include "uigeometryanimation.h"
#include "uiutils.h"
#include "uistyle.h"
#ifdef _WIN32
# include "uithread.h"
# include <windowsx.h>
# include <CommCtrl.h>
#else
# include <cmath>
#endif

#ifdef _WIN32
static double GetTopWindowLogicalDpi(HWND hWnd)
{
    HWND hwndRoot = GetAncestor(hWnd, GA_ROOT);
    if (HMODULE module = GetModuleHandleA("user32")) {
        UINT(WINAPI *_GetDpiForWindow)(HWND) = NULL;
        *(FARPROC*)&_GetDpiForWindow = GetProcAddress(module, "GetDpiForWindow");
        if (_GetDpiForWindow)
            return (double)_GetDpiForWindow(hwndRoot)/96;
    }
    HDC hdc = GetDC(hWnd);
    double dpi = (double)GetDeviceCaps(hdc, LOGPIXELSX)/96;
    ReleaseDC(NULL, hdc);
    return dpi;
}
#endif

UIWidget::UIWidget(UIWidget *parent) :
    UIWidget(parent, ObjectType::WidgetType)
{}

UIWidget::UIWidget(UIWidget *parent, ObjectType type, PlatformWindow hWindow, const Rect &rc) :
    UIObject(type, parent),
    UIDrawningSurface(),
    m_hFont(nullptr),
    m_hWindow(hWindow),
#ifdef _WIN32
    m_root_hWnd(nullptr),
    m_root_is_layered(false),
#endif    
    m_layout(nullptr),
    m_dpi_ratio(1.0),
    m_corners(UIDrawingEngine::CornerAll),
    m_disabled(false),
    m_rtl(UIApplication::instance()->layoutDirection() == UIApplication::RightToLeft),
    m_drag_handler(nullptr),
    m_geometry_animation(nullptr),
    m_font_size(10.0),
    m_is_created(false),
    m_is_active(false),
    m_is_destroyed(false),
    m_is_class_destroyed(false),
    m_mouse_entered(false)
{
    m_size_behaviors[SizePolicy::HSizeBehavior] = SizePolicy::Expanding;
    m_size_behaviors[SizePolicy::VSizeBehavior] = SizePolicy::Expanding;
    if (hWindow) {
#ifdef _WIN32
        LONG style = ::GetWindowLong(m_hWindow, GWL_STYLE) | WS_CHILD;
        ::SetWindowLong(m_hWindow, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
        SetParent(m_hWindow, parent->platformWindow());
#else
        if (parent)
            gtk_layout_put((GtkLayout*)parent->gtkLayout(), m_hWindow, 0, 0);
#endif
    } else {
        UIApplication::instance()->registerWidget(this, type, rc);
#ifdef _WIN32
        m_dpi_ratio = GetTopWindowLogicalDpi(m_hWindow);
#endif
    }
#ifdef _WIN32
    m_root_hWnd = GetAncestor(m_hWindow, GA_ROOT);
    m_root_is_layered = (m_root_hWnd && (GetWindowLong(m_root_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED));
#endif
    if (!hWindow) {
        setFont(UIApplication::instance()->font(), UIApplication::instance()->fontPointSize());
        UIApplication::instance()->style()->registerWidget(this);
    }
}

UIWidget::~UIWidget()
{
    if (m_drag_handler) {
        delete m_drag_handler; m_drag_handler = nullptr;
    }
    if (m_geometry_animation) {
        delete m_geometry_animation; m_geometry_animation = nullptr;
    }
    UIApplication::instance()->style()->unregisterWidget(this);
    m_is_class_destroyed = true;
    if (m_layout) {
        if (UIUtils::isAllocOnHeap(m_layout))
            delete m_layout;
        m_layout = nullptr;
    }
#ifdef _WIN32
    if (!m_is_destroyed)
        DestroyWindow(m_hWindow);
    if (m_hFont)
        DeleteObject(m_hFont);
#else
    if (!m_is_destroyed)
        gtk_widget_destroy(m_hWindow);
    if (m_hFont)
        pango_font_description_free(m_hFont);
#endif
}

void UIWidget::setObjectGroupId(const tstring &id)
{
    UIObject::setObjectGroupId(id);
    applyStyle();
}

void UIWidget::setGeometry(int x, int y, int width, int height)
{
#ifdef _WIN32
    SetWindowPos(m_hWindow, NULL, x, y, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
#else
    m_size = Size(width, height);
    if (width > -1 && height > -1)
        gtk_widget_set_size_request(m_hWindow, width, height);
    if (UIWidget *parent = parentWidget()) {
        int final_x = x;
        if (m_rtl) {
            int layout_width = parent->size().width;
            final_x = layout_width - x - width;
        }
        gtk_layout_move((GtkLayout*)parent->gtkLayout(), m_hWindow, final_x, y);
    }
    if (m_layout)
        m_layout->onResize(width, height);
    for (auto it = m_resize_callbacks.begin(); it != m_resize_callbacks.end(); it++)
        if (it->second)
            (it->second)(width, height);
#endif
}

void UIWidget::setDisabled(bool disable)
{
    m_disabled = disable;
    palette()->setCurrentState(disable ? Palette::Disabled : Palette::Normal);
    update();
}

void UIWidget::close()
{
#ifdef _WIN32
    PostMessage(m_hWindow, WM_CLOSE, 0, 0);
#else
    if (GTK_IS_WINDOW(m_hWindow)) {
        gtk_window_close(GTK_WINDOW(m_hWindow));
    } else {
        GdkEvent *ev = gdk_event_new(GDK_DELETE);
        GdkWindow *gdk_wnd = GTK_IS_LAYOUT(m_hWindow) ? gtk_layout_get_bin_window(GTK_LAYOUT(m_hWindow)) : gtk_widget_get_window(m_hWindow);
        ev->any.window = (GdkWindow*)g_object_ref(gdk_wnd);
        ev->any.send_event = TRUE;
        gdk_event_put(ev);
        gdk_event_free(ev);
    }
#endif
}

void UIWidget::move(int x, int y)
{
#ifdef _WIN32
    SetWindowPos(m_hWindow, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
#else
    if (UIWidget *parent = parentWidget()) {
        int final_x = x;
        if (m_rtl) {
            int layout_width = parent->size().width;;
            final_x = layout_width - x - m_size.width;
        }
        gtk_layout_move((GtkLayout*)parent->gtkLayout(), m_hWindow, final_x, y);
    }
#endif
}

void UIWidget::resize(int w, int h)
{
#ifdef _WIN32
    SetWindowPos(m_hWindow, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
#else
    m_size = Size(w, h);
    if (w > -1 && h > -1)
        gtk_widget_set_size_request(m_hWindow, w, h);
    if (m_layout)
        m_layout->onResize(w, h);
    for (auto it = m_resize_callbacks.begin(); it != m_resize_callbacks.end(); it++)
        if (it->second)
            (it->second)(w, h);
#endif
}

UIWidget *UIWidget::parentWidget() const noexcept
{
    return dynamic_cast<UIWidget*>(parent());
}

Size UIWidget::size() const
{
#ifdef _WIN32
    RECT rc;
    GetClientRect(m_hWindow, &rc);
    return Size(rc.right - rc.left, rc.bottom - rc.top);
#else
    return m_size;
#endif
}

void UIWidget::size(int *width, int *height) const
{
#ifdef _WIN32
    RECT rc;
    GetClientRect(m_hWindow, &rc);
    *width = rc.right - rc.left;
    *height =  rc.bottom - rc.top;
#else
    *width = m_size.width;
    *height = m_size.height;
#endif
}

Point UIWidget::pos() const
{
#ifdef _WIN32
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWindow, &wp);
    return Point(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
#else
    GtkAllocation alc;
    gtk_widget_get_allocation(m_hWindow, &alc);
    return Point(alc.x, alc.y);
#endif
}

void UIWidget::updateGeometry()
{
#ifdef _WIN32
    SetWindowPos(m_hWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
#else
    GtkWidget *toplevel = gtk_widget_get_toplevel(m_hWindow);
    if (toplevel)
        gtk_widget_queue_resize(toplevel);
#endif
}

UIGeometryAnimation *UIWidget::geometryAnimation()
{
    return m_geometry_animation;
}

void UIWidget::setGeometryAnimation(UIGeometryAnimation *geometry_animation)
{
    m_geometry_animation = geometry_animation;
}

UIDragHandler *UIWidget::dragHandler()
{
    return m_drag_handler;
}

void UIWidget::setDragHandler(UIDragHandler *drag_handler)
{
    m_drag_handler = drag_handler;
}

void UIWidget::applyStyle()
{
    UIApplication::instance()->style()->setStyle(this);
}

void UIWidget::setSizePolicy(SizePolicy::Properties property, int val)
{
    m_size_behaviors[property] = val;
}

void UIWidget::setFont(const tstring &font, double fontPointSize)
{
    m_font_size = fontPointSize > 0 ? fontPointSize : 10.0;
#ifdef _WIN32
    m_font = font.empty() ? L"Arial" : font;
    if (m_hFont) {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
    int h = -round((double)MulDiv(m_font_size * 10, m_dpi_ratio * 96, 72)/10);
    m_hFont = CreateFont(h, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, m_font.c_str());
#else
    m_font = font.empty() ? "Helvetica" : font;
    if (m_hFont) {
        pango_font_description_free(m_hFont);
        m_hFont = nullptr;
    }
    m_hFont = pango_font_description_new();
    pango_font_description_set_family(m_hFont, m_font.c_str());
    pango_font_description_set_size(m_hFont, m_font_size * PANGO_SCALE);
#endif
}

void UIWidget::setBaseSize(int w, int h)
{
    m_base_size = Size(w, h);
    resize(round(w * m_dpi_ratio), round(h * m_dpi_ratio));
}

void UIWidget::setCorners(unsigned char corner)
{
    m_corners = corner;
}

void UIWidget::setAcceptDrops(bool accept)
{
#ifdef _WIN32
    DragAcceptFiles(m_hWindow, accept);
#else
    if (accept) {
        const GtkTargetEntry targets[] = {
            {(gchar*)"text/uri-list", 0, 0}
        };
        gtk_drag_dest_set(m_hWindow, GTK_DEST_DEFAULT_ALL, targets, G_N_ELEMENTS(targets), GDK_ACTION_COPY);
    } else {
        gtk_drag_dest_unset(m_hWindow);
    }
#endif
}

void UIWidget::show()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_SHOWNORMAL);
    UpdateWindow(m_hWindow);
#else
    gtk_widget_show(m_hWindow);
#endif
}

void UIWidget::hide()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_HIDE);
#else
    gtk_widget_hide(m_hWindow);
#endif
}

void UIWidget::repaint()
{
#ifdef _WIN32
    if (IsWindowVisible(m_hWindow)) {
        if (m_root_is_layered) {
            SendMessage(m_root_hWnd, WM_UPDATE_LAYERED_WINDOW, 0, 0);
        } else
            RedrawWindow(m_hWindow, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT | RDW_UPDATENOW);
    }
#else
    gtk_widget_queue_draw(m_hWindow);
    gdk_window_process_all_updates();
#endif
}

void UIWidget::update()
{
#ifdef _WIN32
    if (IsWindowVisible(m_hWindow)) {
        if (m_root_is_layered) {
            PostMessage(m_root_hWnd, WM_UPDATE_LAYERED_WINDOW, 0, 0);
        } else
            RedrawWindow(m_hWindow, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT | RDW_FRAME);
    }
#else
    gtk_widget_queue_draw(m_hWindow);
#endif
}

void UIWidget::bringAboveSiblings()
{
#ifdef _WIN32
    SetWindowPos(m_hWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
    gtk_widget_set_child_visible(m_hWindow, FALSE);
    gtk_widget_set_child_visible(m_hWindow, TRUE);
#endif
}

void UIWidget::setLayout(UILayout *layout)
{
    if (m_layout) {
        // TODO: error: trying to add a layout when the widget contains a layout
    } else {
        m_layout = layout;
    }
}

bool UIWidget::isCreated()
{
    return m_is_created;
}

bool UIWidget::isActive()
{
    return m_is_active;
}

bool UIWidget::underMouse()
{
#ifdef _WIN32
    POINT pt;
    GetCursorPos(&pt);
    return WindowFromPoint(pt) == m_hWindow;
#else
    GdkDisplay *dsp = gdk_display_get_default();
    GdkDeviceManager *dm = gdk_display_get_device_manager(dsp);
    GdkDevice *dev = gdk_device_manager_get_client_pointer(dm);
    gint x, y;
    gdk_device_get_position(dev, NULL, &x, &y);
    return gdk_device_get_window_at_position(dev, &x, &y) == gtk_layout_get_bin_window(GTK_LAYOUT(m_hWindow));
#endif
}

void UIWidget::grabMouse()
{
#ifdef _WIN32
    SetCapture(m_hWindow);
#else
    GdkDisplay *dsp = gtk_widget_get_display(m_hWindow);
    GdkDeviceManager *dm = gdk_display_get_device_manager(dsp);
    GdkDevice *dev = gdk_device_manager_get_client_pointer(dm);
    if (GdkWindow *gdk_wnd = gtk_layout_get_bin_window(GTK_LAYOUT(m_hWindow))) {
        GdkEventMask mask = (GdkEventMask)(GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
        gdk_device_grab(dev, gdk_wnd, GDK_OWNERSHIP_NONE, FALSE, mask, NULL, GDK_CURRENT_TIME);
    }
#endif
}

void UIWidget::ungrabMouse()
{
#ifdef _WIN32
    ReleaseCapture();
#else
    GdkDisplay *dsp = gtk_widget_get_display(m_hWindow);
    GdkDeviceManager *dm = gdk_display_get_device_manager(dsp);
    GdkDevice *dev = gdk_device_manager_get_client_pointer(dm);
    gdk_device_ungrab(dev, GDK_CURRENT_TIME);
#endif
}

int UIWidget::sizePolicy(SizePolicy::Properties property)
{
    return m_size_behaviors[property];
}

double UIWidget::dpiRatio()
{
    return m_dpi_ratio;
}

UILayout *UIWidget::layout() const noexcept
{
    return m_layout;
}

PlatformWindow UIWidget::platformWindow() const noexcept
{
    return m_hWindow;
}

UIWidget* UIWidget::topLevelWidget() const noexcept
{
    const UIWidget *top = this;
    while (const UIWidget *parent = top->parentWidget()) {
        top = parent;
    }
    return const_cast<UIWidget*>(top);
}

UIWidget *UIWidget::widgetFromHwnd(UIWidget *parent, PlatformWindow hwnd)
{
    return new UIWidget(parent, ObjectType::WidgetType, hwnd);
}

int UIWidget::onResize(const FnVoidIntInt &callback)
{
    m_resize_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onMove(const FnVoidIntInt &callback)
{
    m_move_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onAboutToDestroy(const FnVoidVoid &callback)
{
    m_destroy_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onCreate(const FnVoidVoid &callback)
{
    m_create_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onActivationChanged(const FnVoidBool &callback)
{
    m_activation_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onClose(const FnVoidBoolPtr &callback)
{
    m_close_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onDropFiles(const FnVoidVecStr &callback)
{
    m_drop_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int UIWidget::onContextMenu(const FnVoidIntInt &callback)
{
    m_context_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

void UIWidget::onInvokeMethod(long long wParam)
{
    if (std::function<void()> *func = (std::function<void()>*)wParam) {
        if (*func)
            (*func)();
        delete func;
    }
}

void UIWidget::disconnect(int connectionId)
{
    {
        auto it = m_resize_callbacks.find(connectionId);
        if (it != m_resize_callbacks.end()) {
            m_resize_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_move_callbacks.find(connectionId);
        if (it != m_move_callbacks.end()) {
            m_move_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_destroy_callbacks.find(connectionId);
        if (it != m_destroy_callbacks.end()) {
            m_destroy_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_create_callbacks.find(connectionId);
        if (it != m_create_callbacks.end()) {
            m_create_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_activation_callbacks.find(connectionId);
        if (it != m_activation_callbacks.end()) {
            m_activation_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_close_callbacks.find(connectionId);
        if (it != m_close_callbacks.end()) {
            m_close_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_drop_callbacks.find(connectionId);
        if (it != m_drop_callbacks.end()) {
            m_drop_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_context_callbacks.find(connectionId);
        if (it != m_context_callbacks.end()) {
            m_context_callbacks.erase(it);
            return;
        }
    }
}

#ifdef _WIN32
bool UIWidget::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_ACTIVATE:
        break;

    case WM_CREATE: {
        UIThread::invoke(this, [=]() {
            onAfterCreated();
        });
        m_is_created = true;
        for (auto it = m_create_callbacks.begin(); it != m_create_callbacks.end(); it++)
            if (it->second)
                (it->second)();
        break;
    }

    case WM_SIZE:
        if (m_layout)
            m_layout->onResize(LOWORD(lParam), HIWORD(lParam), m_dpi_ratio);
        for (auto it = m_resize_callbacks.begin(); it != m_resize_callbacks.end(); it++)
            if (it->second)
                (it->second)(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_MOVE:
        for (auto it = m_move_callbacks.begin(); it != m_move_callbacks.end(); it++)
            if (it->second)
                (it->second)(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_PAINT_LAYERED_CHILD:
    case WM_PAINT: {
        UIDrawingEngine *de = engine();
        RECT rc;
        GetClientRect(m_hWindow, &rc);
        if (msg == WM_PAINT)
            de->Begin(this, m_hWindow, &rc, m_dpi_ratio);
        else
            de->LayeredChildBegin(this, m_hWindow, &rc, m_dpi_ratio);

        if (metrics()->value(Metrics::BorderRadius) == 0)
            de->DrawFlatRect(msg == WM_PAINT);
        else
            de->DrawRoundedRect(m_corners, 0, msg == WM_PAINT);

        onPaint(rc);

        if (msg == WM_PAINT)
            de->End();
        else
            de->LayeredChildEnd();
        *result = FALSE;
        return true;
    }

    case WM_DPICHANGED_NOTIFY: {
        m_dpi_ratio = (double)wParam/96;
        if (m_hFont) {
            DeleteObject(m_hFont);
            m_hFont = nullptr;
        }
        int h = -round((double)MulDiv(m_font_size * 10, m_dpi_ratio * 96, 72)/10);
        m_hFont = CreateFont(h, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, m_font.c_str());
        resize(round(m_base_size.width * m_dpi_ratio), round(m_base_size.height * m_dpi_ratio));
        break;
    }

    case WM_ERASEBKGND: {
        // if (UIUtils::winVersion() >= UIUtils::WinVer::Win10) {
        //     HDC hdc = (HDC)wParam;
        //     RECT rc;
        //     GetClientRect(m_hWindow, &rc);
        //     HBRUSH hBrush = CreateSolidBrush(palette()->color(Palette::Background));
        //     FillRect(hdc, &rc, hBrush);
        //     DeleteObject(hBrush);
        // }
        *result = TRUE;
        return true;
    }

    case WM_LBUTTONUP: {
        SetFocus(m_hWindow);
        if (m_drag_handler) {
            m_drag_handler->handleButtonUpEvent();
        }
        break;
    }

    case WM_RBUTTONUP: {
        POINT pos{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        ClientToScreen(m_hWindow, &pos);
        for (auto it = m_context_callbacks.begin(); it != m_context_callbacks.end(); it++) {
            if (it->second)
                (it->second)(pos.x, pos.y);
        }
        break;
    }

    case WM_LBUTTONDOWN:
        if (m_drag_handler) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            m_drag_handler->handleButtonDownEvent(x, y);
        }
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN: {
        if (m_root_hWnd && m_root_hWnd != m_hWindow)
            PostMessage(m_root_hWnd, WM_CHILD_BUTTONDOWN_NOTIFY, wParam, lParam);
        break;
    }

    case WM_KEYDOWN: {
        if (m_root_hWnd && m_root_hWnd != m_hWindow)
            PostMessage(m_root_hWnd, WM_CHILD_KEYDOWN_NOTIFY, wParam, lParam);
        break;
    }

    case WM_MOUSEMOVE: {
        if (!m_mouse_entered) {
            m_mouse_entered = true;
            PostMessage(m_hWindow, WM_MOUSEENTER, 0, 0);
        }
        // add here impl onMouseMove
        if (m_drag_handler && wParam & MK_LBUTTON) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            m_drag_handler->handleMouseMoveEvent(x, y);
        }

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWindow;
        tme.dwFlags = TME_LEAVE /*| TME_HOVER*/;
        tme.dwHoverTime = HOVER_DEFAULT;
        _TrackMouseEvent(&tme);
        break;
    }

    case WM_NCMOUSEMOVE: {
        if (!m_mouse_entered) {
            m_mouse_entered = true;
            PostMessage(m_hWindow, WM_MOUSEENTER, 0, 0);
        }
        // add here impl onMouseMove
        break;
    }

    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE: {
        if (m_mouse_entered) {
            m_mouse_entered = false;
        }
        break;
    }

    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        std::vector<tstring> paths;
        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        for (UINT i = 0; i < fileCount; ++i) {
            TCHAR filePath[MAX_PATH];
            if (DragQueryFile(hDrop, i, filePath, MAX_PATH)) {
                paths.emplace_back(filePath);
            }
        }
        DragFinish(hDrop);
        for (auto it = m_drop_callbacks.begin(); it != m_drop_callbacks.end(); it++)
            if (it->second)
                (it->second)(paths);
        *result = FALSE;
        return true;
    }

    case WM_CLOSE: {
        bool accept = true;
        for (auto it = m_close_callbacks.begin(); it != m_close_callbacks.end(); it++)
            if (it->second)
                (it->second)(&accept);
        if (accept)
            DestroyWindow(m_hWindow);
        *result = TRUE;
        return true;
    }

    case WM_DESTROY: {
        m_is_destroyed = true;
        for (auto it = m_destroy_callbacks.begin(); it != m_destroy_callbacks.end(); it++)
            if (it->second)
                (it->second)();

        SetWindowLongPtr(m_hWindow, GWLP_USERDATA, 0);
        if (!m_is_class_destroyed) {
            if (UIUtils::isAllocOnHeap(this)) {
                delete this;
            }
        }
        break;
    }

    case WM_PARENT_ACTIVATION_NOTIFY: {
        m_is_active = LOWORD(wParam);
        for (auto it = m_activation_callbacks.begin(); it != m_activation_callbacks.end(); it++)
            if (it->second)
                (it->second)(m_is_active);
        break;
    }

    case WM_INVOKEMETHOD: {
        onInvokeMethod(wParam);
        break;
    }    

    default:
        break;
    }
    return false;
}

LRESULT UIWidget::checkInputRegion(LPARAM lParam, const RECT &rc)
{
    POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(m_root_is_layered ? m_root_hWnd : m_hWindow, &pt);
    return PtInRect(&rc, pt) ? HTCLIENT : HTTRANSPARENT;
}
#else
bool UIWidget::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_HOOKED_REALIZE: {
        m_is_created = true;
        for (auto it = m_create_callbacks.begin(); it != m_create_callbacks.end(); it++)
            if (it->second)
                (it->second)();
        return false;
    }

    case GDK_CONFIGURE: {
        GdkEventConfigure *cev = (GdkEventConfigure*)param;
        if (cev->x != m_pos.x || cev->y != m_pos.y) {
            m_pos = Point(cev->x, cev->y);
            for (auto it = m_move_callbacks.begin(); it != m_move_callbacks.end(); it++)
                if (it->second)
                    (it->second)(cev->x, cev->y);
        }
        return false;
    }

    case GDK_HOOKED_DRAW: {
        UIDrawingEngine *de = engine();
        Rect rc(0, 0, m_size.width, m_size.height);
        de->Begin(this, (cairo_t*)param, &rc);
        if (metrics()->value(Metrics::BorderRadius) == 0)
            de->DrawFlatRect();
        else
            de->DrawRoundedRect(m_corners);

        onPaint(rc);

        de->End();
        return false;
    }

    case GDK_BUTTON_PRESS: {
        if (!m_disabled) {
            GdkEventButton *bev = (GdkEventButton*)param;
            GtkWidget *root = gtk_widget_get_toplevel(m_hWindow);
            if (m_drag_handler && bev->button == GDK_BUTTON_PRIMARY) {
                m_drag_handler->handleButtonDownEvent(bev->x_root, bev->y_root);
            }
            if (root && root != m_hWindow)
                UIApplication::sendEvent(root, GDK_CHILD_BUTTONDOWN_NOTIFY, &bev->button);
            gtk_widget_grab_focus(m_hWindow);
        }
        return true;
    }

    case GDK_BUTTON_RELEASE: {
        GdkEventButton *bev = (GdkEventButton*)param;
        if (bev->button == GDK_BUTTON_PRIMARY) {
            if (m_drag_handler) {
                m_drag_handler->handleButtonUpEvent();
            }
        } else
        if (bev->button == GDK_BUTTON_SECONDARY) {
            for (auto it = m_context_callbacks.begin(); it != m_context_callbacks.end(); it++) {
                if (it->second)
                    (it->second)(bev->x_root, bev->y_root);
            }
        }
        return true;
    }

    case GDK_MOTION_NOTIFY: {
        GdkEventMotion *mev = (GdkEventMotion*)param;
        GdkModifierType state = (GdkModifierType)mev->state;
        if (m_drag_handler && state & GDK_BUTTON1_MASK) {
            m_drag_handler->handleMouseMoveEvent(mev->x_root, mev->y_root);
        }
        return true;
    }

    case GDK_ENTER_NOTIFY: {
        if (!m_mouse_entered)
            m_mouse_entered = true;
        break;
    }

    case GDK_LEAVE_NOTIFY: {
        if (m_mouse_entered)
            m_mouse_entered = false;
        break;
    }

    case GDK_HOOKED_DROPFILES: {
        DropFilesInfo *dfi = (DropFilesInfo*)param;
        std::vector<tstring> paths;
        gchar **uris = gtk_selection_data_get_uris(dfi->sel_data);
        if (uris) {
            for (int i = 0; uris[i] != NULL; i++) {
                gchar *filePath = g_filename_from_uri(uris[i], NULL, NULL);
                if (filePath) {
                    paths.emplace_back(filePath);
                    g_free(filePath);
                }
            }
            g_strfreev(uris);
        }
        gtk_drag_finish(dfi->context, TRUE, FALSE, dfi->time);
        for (auto it = m_drop_callbacks.begin(); it != m_drop_callbacks.end(); it++)
            if (it->second)
                (it->second)(paths);
        return false;
    }

    case GDK_HOOKED_SIZE_ALLOC: {
        GtkAllocation *alc = (GtkAllocation*)param;
        m_size = Size(alc->width, alc->height);
        for (auto it = m_resize_callbacks.begin(); it != m_resize_callbacks.end(); it++)
            if (it->second)
                (it->second)(alc->width, alc->height);
        break;
    }

    case GDK_DELETE: {
        bool accept = true;
        for (auto it = m_close_callbacks.begin(); it != m_close_callbacks.end(); it++)
            if (it->second)
                (it->second)(&accept);
        return !accept;
    }

    case GDK_HOOKED_DESTROY: {
        m_is_destroyed = true;
        for (auto it = m_destroy_callbacks.begin(); it != m_destroy_callbacks.end(); it++)
            if (it->second)
                (it->second)();

        g_object_set_data(G_OBJECT(m_hWindow), "UIWidget", NULL);
        if (!m_is_class_destroyed) {
            if (UIUtils::isAllocOnHeap(this)) {
                delete this;
            }
        }
        break;
    }

    case GDK_PARENT_ACTIVATION_NOTIFY: {
        bool *is_active = (bool*)param;
        m_is_active = *is_active;
        for (auto it = m_activation_callbacks.begin(); it != m_activation_callbacks.end(); it++)
            if (it->second)
                (it->second)(m_is_active);
        break;
    }

    case GDK_INVOKEMETHOD: {
        onInvokeMethod((long long)param);
        break;
    }

    default:
        break;
    }
    return false;
}

void UIWidget::updateInputRegion(const RECT &rc)
{
    GdkWindow *gdk_wnd = gtk_widget_get_window(m_hWindow);
    cairo_region_t *rg = cairo_region_create();
    GdkRectangle grc = {rc.x, rc.y, rc.width, rc.height};
    cairo_region_union_rectangle(rg, &grc);
    gdk_window_input_shape_combine_region(gdk_wnd, rg, 0, 0);
    cairo_region_destroy(rg);
}

GtkWidget* UIWidget::gtkLayout()
{
    return m_hWindow;
}
#endif

void UIWidget::onPaint(const RECT&)
{

}

void UIWidget::onAfterCreated()
{

}

void UIWidget::setPlatformWindow(PlatformWindow hWindow)
{
    m_hWindow = hWindow;
}
