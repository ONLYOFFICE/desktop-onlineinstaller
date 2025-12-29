#include "uiapplication.h"
#include "uiwidget.h"
#include "uistyle.h"
#ifdef _WIN32
# include <gdiplus.h>
#else
# include "uiutils.h"

struct EventData {
    GtkWidget *widget;
    uint event_type;
    void *param;
};
#endif


class UIApplication::UIApplicationPrivate
{
public:
    UIApplicationPrivate();
    ~UIApplicationPrivate();

    FontInfo fontInfo;
    UIStyle  *style;
#ifdef _WIN32
    ULONG_PTR gdi_token;
    HINSTANCE hInstance;
    static HHOOK hHook;
    ATOM registerClass(LPCWSTR className, HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
#else
    int exit_code;
    void registerEvents(UIWidget *wgt);
    static gboolean EventProc(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void EventAfterProc(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void RealizeEventProc(GtkWidget *wgt, gpointer data);
    static gboolean ConfigEventProc(GtkWidget *wgt, GdkEventConfigure *ev, gpointer data);
    static void SizeEventProc(GtkWidget *wgt, GtkAllocation *alc, gpointer data);
    static gboolean DrawProc(GtkWidget *wgt, cairo_t *cr, gpointer data);
    static void DestroyProc(GtkWidget *wgt, gpointer data);
    static void DropFilesProc(GtkWidget *wgt, GdkDragContext *ctxt, gint x, gint y,
                              GtkSelectionData *sel_data, guint info, guint time, gpointer data);
    static gboolean IdleProc(gpointer data);
#endif
    LayoutDirection layoutDirection;
    int windowId;
};

#ifdef _WIN32
HHOOK UIApplication::UIApplicationPrivate::hHook = nullptr;
#endif

UIApplication::UIApplicationPrivate::UIApplicationPrivate() :
    style(&UIStyle::instance()),
#ifdef _WIN32
    gdi_token(0),
    hInstance(nullptr),
#else
    exit_code(0),
#endif
    layoutDirection(LayoutDirection::LeftToRight),
    windowId(0)
{
#ifdef _WIN32
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdi_token, &gdiplusStartupInput, nullptr);
    hHook = SetWindowsHookExW(WH_CBT, CBTProc, nullptr, GetCurrentThreadId());
#endif
}

UIApplication::UIApplicationPrivate::~UIApplicationPrivate()
{
#ifdef _WIN32
    UnhookWindowsHookEx(hHook);
    Gdiplus::GdiplusShutdown(gdi_token);
#endif
}

#ifdef _WIN32
ATOM UIApplication::UIApplicationPrivate::registerClass(LPCWSTR className, HINSTANCE hInstance)
{
    WNDCLASSEX wcx;
    memset(&wcx, 0, sizeof(wcx));
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    // wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    // wcx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wcx.lpszClassName = className;
    wcx.hbrBackground = nullptr;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassEx(&wcx);
}

LRESULT CALLBACK UIApplication::UIApplicationPrivate::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE) {
        if (CREATESTRUCT *cs = (CREATESTRUCT*)lParam) {
            if (UIWidget *wgt = (UIWidget*)cs->lpCreateParams) {
                wgt->setPlatformWindow(hWnd);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
                LRESULT result = 0;
                if (wgt->event(msg, wParam, lParam, &result))
                    return result;
            }
        }
    } else
    if (UIWidget *wgt = (UIWidget*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
         LRESULT result = 0;
         if (wgt->event(msg, wParam, lParam, &result))
             return result;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK UIApplication::UIApplicationPrivate::CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_ACTIVATE) {
        HWND hWnd = (HWND)wParam;
        if (GetProp(hWnd, L"BlockActivation")) {
            return 1;
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
#else

void UIApplication::UIApplicationPrivate::registerEvents(UIWidget *wgt)
{
    PlatformWindow hwnd = wgt->platformWindow();
    g_signal_connect(G_OBJECT(hwnd), "event", G_CALLBACK(EventProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "event-after", G_CALLBACK(EventAfterProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "realize", G_CALLBACK(RealizeEventProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "configure-event", G_CALLBACK(ConfigEventProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "size-allocate", G_CALLBACK(SizeEventProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "draw", G_CALLBACK(DrawProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "destroy", G_CALLBACK(DestroyProc), wgt);
    g_signal_connect(G_OBJECT(hwnd), "drag-data-received", G_CALLBACK(DropFilesProc), wgt);
}

gboolean UIApplication::UIApplicationPrivate::EventProc(GtkWidget*, GdkEvent *ev, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        return w->event(ev->type, (void*)ev);
    }
    return FALSE;
}

void UIApplication::UIApplicationPrivate::EventAfterProc(GtkWidget*, GdkEvent *ev, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        switch (ev->type) {
        case GDK_CONFIGURE: {
            w->event(GDK_HOOKED_CONFIGURE_AFTER, (void*)ev);
            break;
        }
        case GDK_MAP: {
            w->event(GDK_HOOKED_MAP_AFTER, (void*)ev);
            break;
        }
        case GDK_FOCUS_CHANGE: {
            w->event(GDK_HOOKED_FOCUS_CHANGE_AFTER, (void*)ev);
            break;
        }
        case GDK_WINDOW_STATE: {
            w->event(GDK_HOOKED_WINDOW_STATE_AFTER, (void*)ev);
            break;
        }
        case GDK_BUTTON_PRESS: {
            w->event(GDK_HOOKED_BUTTON_PRESS_AFTER, (void*)ev);
            break;
        }
        case GDK_DOUBLE_BUTTON_PRESS: {
            w->event(GDK_HOOKED_DBLBUTTON_PRESS_AFTER, (void*)ev);
            break;
        }
        default:
            break;
        }
    }
}

void UIApplication::UIApplicationPrivate::RealizeEventProc(GtkWidget*, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        w->event(GDK_HOOKED_REALIZE, NULL);
    }
}

gboolean UIApplication::UIApplicationPrivate::ConfigEventProc(GtkWidget*, GdkEventConfigure *ev, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        return w->event(GDK_HOOKED_CONFIGURE, ev);
    }
    return FALSE;
}

void UIApplication::UIApplicationPrivate::SizeEventProc(GtkWidget*, GtkAllocation *alc, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        w->event(GDK_HOOKED_SIZE_ALLOC, (void*)alc);
    }
}

gboolean UIApplication::UIApplicationPrivate::DrawProc(GtkWidget*, cairo_t *cr, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        return w->event(GDK_HOOKED_DRAW, cr);
    }
    return FALSE;
}

void UIApplication::UIApplicationPrivate::DestroyProc(GtkWidget*, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        w->event(GDK_HOOKED_DESTROY, NULL);
    }
}

void UIApplication::UIApplicationPrivate::DropFilesProc(GtkWidget*, GdkDragContext *ctxt, gint x, gint y,
                                                        GtkSelectionData *sel_data, guint info, guint time, gpointer data)
{
    if (UIWidget *w = (UIWidget*)data) {
        DropFilesInfo dfi;
        dfi.context = ctxt;
        dfi.x = x;
        dfi.y = y;
        dfi.sel_data = sel_data;
        dfi.info = info;
        dfi.time = time;
        w->event(GDK_HOOKED_DROPFILES, &dfi);
    }
}

gboolean UIApplication::UIApplicationPrivate::IdleProc(gpointer data)
{
    if (EventData *ev_data = (EventData*)data) {
        UIWidget *w = (UIWidget*)g_object_get_data(G_OBJECT(ev_data->widget), "UIWidget");
        if (w) {
            w->event(ev_data->event_type, ev_data->param);
        }
        delete ev_data;
    }
    return G_SOURCE_REMOVE;
}
#endif

UIApplication *UIApplication::inst = nullptr;

#ifdef _WIN32
UIApplication::UIApplication(HINSTANCE hInstance, PWSTR cmdline, int cmdshow) :
#else
UIApplication::UIApplication(int argc, char *argv[]) :
#endif
    UIApplication()
{
#ifdef _WIN32
    d_ptr->hInstance = hInstance;
    if (!d_ptr->hInstance)
        d_ptr->hInstance = GetModuleHandle(NULL);
#else
    gtk_init(&argc, &argv);
#endif
    inst = this;
}

UIApplication::UIApplication() :
    UIObject(ObjectType::ApplicationType, nullptr),
    d_ptr(new UIApplicationPrivate)
{

}

UIApplication *UIApplication::instance()
{
    return inst;
}

#ifdef _WIN32
HINSTANCE UIApplication::moduleHandle()
{
    return d_ptr->hInstance;
}
#endif

void UIApplication::setLayoutDirection(LayoutDirection layoutDirection)
{
#ifdef __linux__
    if (layoutDirection == LayoutDirection::RightToLeft)
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
#endif
    d_ptr->layoutDirection = layoutDirection;
}

void UIApplication::setFont(const FontInfo &fontInfo)
{
    d_ptr->fontInfo = fontInfo;
}

UIApplication::LayoutDirection UIApplication::layoutDirection() const
{
    return d_ptr->layoutDirection;
}

FontInfo UIApplication::font() const
{
    return d_ptr->fontInfo;
}

UIStyle* UIApplication::style()
{
    return d_ptr->style;
}

UIApplication::~UIApplication()
{
    delete d_ptr, d_ptr = nullptr;
}

int UIApplication::exec()
{
#ifdef _WIN32
    MSG msg;
    BOOL res;
    while ((res = GetMessage(&msg, NULL, 0, 0)) != 0 && res != -1) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
#else
    gtk_main();
    return d_ptr->exit_code;
#endif
}

void UIApplication::exit(int code)
{
#ifdef _WIN32
    PostQuitMessage(code);
#else
    d_ptr->exit_code = code;
    gtk_main_quit();
#endif
}

#ifdef __linux__
void UIApplication::postEvent(GtkWidget *widget, uint event_type, void *param)
{
    EventData *ev_data = new EventData;
    ev_data->widget = widget;
    ev_data->event_type = event_type;
    ev_data->param = param;
    g_idle_add((GSourceFunc)UIApplicationPrivate::IdleProc, (gpointer)ev_data);
}   // NOLINT

bool UIApplication::sendEvent(GtkWidget *widget, uint event_type, void *param)
{
    UIWidget *wgt = (UIWidget*)g_object_get_data(G_OBJECT(widget), "UIWidget");
    return wgt ? wgt->event(event_type, param) : false;
}
#endif

void UIApplication::registerWidget(UIWidget *wgt, ObjectType objType, const Rect &rc)
{
#ifdef _WIN32
    std::wstring className;
    DWORD style = WS_CLIPCHILDREN;
    DWORD exStyle = 0;
    HWND hWndParent = wgt->parentWidget() ? wgt->parentWidget()->platformWindow() : HWND_DESKTOP;

    switch (objType) {
    case ObjectType::WindowType:
        className = L"MainWindow_" + std::to_wstring(++d_ptr->windowId);
        style |= WS_OVERLAPPEDWINDOW;
        exStyle |= WS_EX_APPWINDOW /*| WS_EX_COMPOSITED | WS_EX_LAYERED*/;
        exStyle |= d_ptr->layoutDirection == LayoutDirection::RightToLeft ? WS_EX_LAYOUTRTL : 0;
        break;    

    case ObjectType::DialogType:
        className = L"Dialog_" + std::to_wstring(++d_ptr->windowId);
        style |= WS_CAPTION | WS_SYSMENU /*| DS_MODALFRAME*/;
        exStyle |= WS_EX_DLGMODALFRAME /*| WS_EX_COMPOSITED | WS_EX_LAYERED*/;
        exStyle |= d_ptr->layoutDirection == LayoutDirection::RightToLeft ? WS_EX_LAYOUTRTL : 0;
        break;

    case ObjectType::PopupType:
        className = L"Popup_" + std::to_wstring(++d_ptr->windowId);
        style |= WS_POPUP;
        exStyle |= WS_EX_TOOLWINDOW | WS_EX_LAYERED /*| WS_EX_NOACTIVATE*/;
        exStyle |= d_ptr->layoutDirection == LayoutDirection::RightToLeft ? WS_EX_LAYOUTRTL : 0;
        break;

    case ObjectType::WidgetType:
    default:
        className = L"Widget_" + std::to_wstring(++d_ptr->windowId);
        style |= WS_CHILD /*| WS_CLIPSIBLINGS */;
        // exStyle |= WS_EX_TRANSPARENT;
        break;
    }

    d_ptr->registerClass(className.c_str(), d_ptr->hInstance);
    CreateWindowExW(exStyle, className.c_str(), L"", style, rc.x, rc.y, rc.width, rc.height, hWndParent, NULL, d_ptr->hInstance, (LPVOID)wgt);
#else
    std::string className;
    GtkWidget *gtkParent = wgt->parentWidget() ? wgt->parentWidget()->platformWindow() : nullptr;
    GtkWidget *gtkWgt = nullptr;

    switch (objType) {
    case ObjectType::WindowType:
        className = "MainWindow_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        if (gtkParent)
            gtk_window_set_transient_for(GTK_WINDOW(gtkWgt), GTK_WINDOW(gtkParent));
        break;

    case ObjectType::DialogType:
        className = "Dialog_" + std::to_string(++d_ptr->windowId);
        // gtkWgt = gtk_dialog_new();
        gtkWgt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        if (gtkParent)
            gtk_window_set_transient_for(GTK_WINDOW(gtkWgt), GTK_WINDOW(gtkParent));
        break;

    case ObjectType::PopupType: {
        className = "Popup_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_window_new(UIUtils::windowServer() == UIUtils::WindowServer::X11 ? GTK_WINDOW_TOPLEVEL : GTK_WINDOW_POPUP);
        if (gtkParent)
            gtk_window_set_transient_for(GTK_WINDOW(gtkWgt), GTK_WINDOW(gtkParent));
        break;
    }

    case ObjectType::WidgetType:
    default:
        className = "Widget_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_layout_new(NULL, NULL);
        gtk_widget_set_size_request(gtkWgt, rc.width, rc.height);
        gtk_widget_add_events(gtkWgt, gtk_widget_get_events(gtkWgt) | GDK_ALL_EVENTS_MASK);
        gtk_widget_set_can_focus(gtkWgt, TRUE);
        if (gtkParent)
            gtk_layout_put((GtkLayout*)wgt->parentWidget()->gtkLayout(), gtkWgt, rc.x, rc.y);
        break;
    }

    g_object_set_data(G_OBJECT(gtkWgt), "UIWidget", (gpointer)wgt);
    wgt->setPlatformWindow(gtkWgt);
    d_ptr->registerEvents(wgt);
    gtk_widget_set_name(gtkWgt, className.c_str());
    // if (d_ptr->layoutDirection == LayoutDirection::RightToLeft)
    //     gtk_widget_set_direction(gtkWgt, GTK_TEXT_DIR_RTL);
#endif
}
