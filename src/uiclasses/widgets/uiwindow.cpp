#include "uiwindow.h"
#include "uidrawningengine.h"
#include "uiutils.h"
#include "uimetrics.h"
#ifdef _WIN32
# include "uithread.h"
# include "uipalette.h"
# include <windowsx.h>
# include <shlwapi.h>
#else
# include "uiapplication.h"
# include <gtk/gtkx.h>
# include <X11/Xlib.h>
# include <X11/Xatom.h>
#endif

#define WINDOW_BORDER_WIDTH 3

#ifdef _WIN32
# define NC_AREA_WIDTH 3
# define DCX_USESTYLE 0x00010000
# ifndef DWMWA_EXTENDED_FRAME_BOUNDS
#  define DWMWA_EXTENDED_FRAME_BOUNDS 9
# endif

using WinVer = UIUtils::WinVer;

static const WinVer ver = UIUtils::winVersion();

struct NotifyParams {
    HWND senderHwnd = nullptr;
    WPARAM wParam = 0;
    LPARAM lParam = 0;
};

static BOOL CALLBACK NcActivationNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    SendMessage(hwnd, WM_PARENT_ACTIVATION_NOTIFY, params->wParam, 0);
    return TRUE;
}

static BOOL CALLBACK SettingsChangeNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    SendMessage(hwnd, WM_SETTINGCHANGE_NOTIFY, params->wParam, 0);
    return TRUE;
}

static BOOL CALLBACK ToplevelButtondownNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    if (hwnd != params->senderHwnd && GetWindow(hwnd, GW_OWNER) == params->senderHwnd) {
        SendMessage(hwnd, WM_TOPLEVEL_BUTTONDOWN_NOTIFY, params->wParam, 0);
    }
    return TRUE;
}

static BOOL CALLBACK ToplevelKeydownNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    if (hwnd != params->senderHwnd && GetWindow(hwnd, GW_OWNER) == params->senderHwnd) {
        SendMessage(hwnd, WM_TOPLEVEL_KEYDONW_NOTIFY, params->wParam, 0);
    }
    return TRUE;
}

static BOOL CALLBACK ToplevelNcActivationNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    if (hwnd != params->senderHwnd && GetWindow(hwnd, GW_OWNER) == params->senderHwnd) {
        SendMessage(hwnd, WM_TOPLEVEL_ACTIVATION_NOTIFY, params->wParam, 0);
    }
    return TRUE;
}

static BOOL CALLBACK DpiChangedNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    SendMessage(hwnd, WM_DPICHANGED_NOTIFY, params->wParam, 0);
    return TRUE;
}

static double GetLogicalDpi(HWND hWnd)
{
    if (HMODULE module = GetModuleHandleA("user32")) {
        UINT(WINAPI *_GetDpiForWindow)(HWND) = NULL;
        *(FARPROC*)&_GetDpiForWindow = GetProcAddress(module, "GetDpiForWindow");
        if (_GetDpiForWindow)
            return (double)_GetDpiForWindow(hWnd)/96;
    }
    HDC hdc = GetDC(hWnd);
    double dpi = (double)GetDeviceCaps(hdc, LOGPIXELSX)/96;
    ReleaseDC(hWnd, hdc);
    return dpi;
}

static Rect availableGeometry(HWND hwnd)
{
    Rect rc;
    if (HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)) {
        MONITORINFO monInfo;
        ZeroMemory(&monInfo, sizeof(monInfo));
        monInfo.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfo(monitor, &monInfo))
            rc = Rect(monInfo.rcWork.left, monInfo.rcWork.top, monInfo.rcWork.right - monInfo.rcWork.left, monInfo.rcWork.bottom - monInfo.rcWork.top);
    }
    return rc;
}

static void GetFrameMetricsForDpi(Margins &frame, double dpi, bool maximized = false)
{
    int row = ver == WinVer::WinXP ? 0 :
                  ver <= WinVer::Win7 ? 1 :
                  ver <= WinVer::Win8_1 ? 2 :
                  ver <= WinVer::Win10 ? 3 : 4;

    int column = dpi <= 1.0 ? 0 :
                     dpi <= 1.25 ? 1 :
                     dpi <= 1.5 ? 2 :
                     dpi <= 1.75 ? 3 :
                     dpi <= 2.0 ? 4 :
                     dpi <= 2.25 ? 5 :
                     dpi <= 2.5 ? 6 :
                     dpi <= 3.0 ? 7 :
                     dpi <= 3.5 ? 8 :
                     dpi <= 4.0 ? 9 :
                     dpi <= 4.5 ? 10 :
                     dpi <= 5.0 ? 11 : 12;

    const int left[5][13] = { // Left margin for scales 100-500%
        {0, 0, 0,  0,  0,  1,  1,  1,  2,  2,  2,  2,  2}, // WinXp: for NC width 3px
        {7, 8, 10, 11, 12, 13, 15, 17, 20, 22, 25, 27, 32}, // WinVista - Win7
        {7, 8, 10, 11, 12, 13, 15, 17, 20, 22, 25, 27, 32}, // Win8 - Win8.1
        {0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win10
        {0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // Win11
    };
    const int top[5][13] = { // Top margin for scales 100-500%
        {0,  0,  0,  0,  0,  1,  1,  1,  2,  2,   2,   2,   2}, // WinXp: for NC width 3px
        {7,  8,  10, 11, 12, 13, 15, 17, 20, 22,  25,  27,  32}, // WinVista - Win7
        {7,  8,  10, 11, 12, 13, 15, 17, 20, 22,  25,  27,  32}, // Win8 - Win8.1
        {31, 38, 45, 52, 58, 65, 72, 85, 99, 112, 126, 139, 167}, // Win10
        {30, 37, 43, 50, 56, 63, 69, 82, 95, 108, 121, 134, 161}  // Win11
    };
    const int left_ofs[5][13] = { // Left offset for scales 100-500%
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinXp
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinVista - Win7
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win8 - Win8.1
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win10
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // Win11
    };
    const int top_ofs[5][13] = { // Top offset for scales 100-500%
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinXp
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinVista - Win7
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win8 - Win8.1
        {8,  9,  11, 12, 13, 14, 16, 18, 21, 24, 27, 30, 36}, // Win10
        {7,  8,  9,  10, 11, 12, 13, 15, 17, 19, 21, 23, 28}  // Win11
    };

    frame.left = left[row][column];
    frame.top = top[row][column];
    if (maximized) {
        frame.left -= left_ofs[row][column];
        frame.top -= top_ofs[row][column];
    }
}

static int Luma(COLORREF color) noexcept
{
    return int(0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color));
}

static COLORREF LighterColor(COLORREF color, WORD factor) noexcept
{
    WORD h = 0, l = 0, s = 0;
    ColorRGBToHLS(color, &h, &l, &s);
    double k = (double)factor/100;
    l = min(240, (unsigned)round(k * l));
    return ColorHLSToRGB(h, l, s);
}

static COLORREF GetBorderColor(bool isActive = true, COLORREF topColor = 0x00ffffff)
{
    int luma = Luma(topColor);
    if (isActive) {
        if (UIUtils::regQueryDwordValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\DWM", L"ColorPrevalence") != 0) {
            DWORD dwcolor = 0;
            BOOL opaque = TRUE;
            HRESULT(WINAPI *DwmGetColorizationColor)(DWORD*, BOOL*) = NULL;
            if (HMODULE module = LoadLibrary(L"dwmapi")) {
                *(FARPROC*)&DwmGetColorizationColor = GetProcAddress(module, "DwmGetColorizationColor");
                if (DwmGetColorizationColor && !SUCCEEDED(DwmGetColorizationColor(&dwcolor, &opaque))) {
                    dwcolor = 0;
                }
                FreeLibrary(module);
                if (dwcolor)
                    return RGB((dwcolor & 0xff0000) >> 16, (dwcolor & 0xff00) >> 8, dwcolor & 0xff);
            }
        } else {
            if (UIUtils::regQueryDwordValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"SystemUsesLightTheme") != 0) {
                std::wstring userSid = UIUtils::currentUserSID();
                if (!userSid.empty()) {
                    userSid.append(L"\\Control Panel\\Desktop");
                    if (UIUtils::regQueryDwordValue(HKEY_USERS, userSid.c_str(), L"AutoColorization") != 0)
                        return LighterColor(topColor, 95);
                }
            }
        }
        int res = -0.002*luma*luma + 0.93*luma + 6;
        return RGB(res, res, res);
    }
    int res = -0.0007*luma*luma + 0.78*luma + 25;
    return RGB(res, res, res);
}

static bool isTaskbarAutoHideOn()
{
    APPBARDATA ABData;
    ABData.cbSize = sizeof(ABData);
    return (SHAppBarMessage(ABM_GETSTATE, &ABData) & ABS_AUTOHIDE) != 0;
}

static bool isThemeActive()
{
    static BOOL(WINAPI *IsThemeActive)() = NULL;
    if (!IsThemeActive) {
        if (HMODULE module = GetModuleHandleA("uxtheme"))
            *(FARPROC*)&IsThemeActive = GetProcAddress(module, "IsThemeActive");
    }
    return IsThemeActive ? (bool)IsThemeActive() : true;
}
#else
# define WINDOW_THIN_BORDER_WIDTH       1
# define WINDOW_CORNER_RADIUS_DEFAULT   0
# define WINDOW_CORNER_RADIUS_GNOME     8
# define WINDOW_CORNER_RADIUS_KDE       6
# define WINDOW_CORNER_RADIUS_CINNAMON  3
# define WINDOW_CORNER_RADIUS_XFCE      4

static void EnumChildLayouts(GtkWidget *widget, gpointer data)
{
    GList **result_list = (GList**)data;
    if (GTK_IS_LAYOUT(widget)) {
        *result_list = g_list_append(*result_list, widget);
    }
    if (GTK_IS_CONTAINER(widget)) {
        gtk_container_foreach(GTK_CONTAINER(widget), EnumChildLayouts, data);
    }
}

static void ParentNotifyProc(GtkWidget *sender, uint event_type, void *param)
{
    GList *children = NULL;
    gtk_container_foreach(GTK_CONTAINER(sender), (GtkCallback)EnumChildLayouts, (gpointer)&children);
    GList *it;
    for (it = children; it != NULL; it = it->next) {
        GtkWidget *child = GTK_WIDGET(it->data);
        UIApplication::sendEvent(child, event_type, param);
    }
    g_list_free(children);
}

static void ToplevelNotifyProc(GtkWidget *sender, uint event_type, void *param)
{
    GList *windows = gtk_window_list_toplevels();
    GList *it;
    for (it = windows; it != NULL; it = it->next) {
        GtkWindow *window = GTK_WINDOW(it->data);
        if (GTK_WIDGET(window) != sender && GTK_WIDGET(gtk_window_get_transient_for(window)) == sender) {
            UIApplication::sendEvent(GTK_WIDGET(window), event_type, param);
        }
    }
    g_list_free(windows);
}

static gboolean onUpdateGeometry(gpointer data)
{
    GtkWidget *wgt = (GtkWidget*)data;
    gtk_widget_queue_resize(wgt); // gtk_widget_queue_allocate(wgt);
    return G_SOURCE_REMOVE;
}
#endif

static void GetFrameBounds(PlatformWindow hWnd, Margins &fr)
{
#ifdef _WIN32
    RECT crc;
    GetClientRect(hWnd, &crc);
    MapWindowPoints(hWnd, HWND_DESKTOP, (POINT*)&crc, 2);
    if (HMODULE module = LoadLibrary(L"dwmapi")) {
        HRESULT(WINAPI *_DwmGetWindowAttribute)(HWND, DWORD, PVOID, DWORD) = NULL;
        *(FARPROC*)&_DwmGetWindowAttribute = GetProcAddress(module, "DwmGetWindowAttribute");
        if (_DwmGetWindowAttribute && SUCCEEDED(_DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &fr, sizeof(RECT)))) {
            fr.left -= crc.left;
            fr.top -= crc.top;
            fr.right -= crc.right;
            fr.bottom -= crc.bottom;
        } else
            fr = Margins();
        FreeLibrary(module);
    }
#else
    if (GdkWindow *gdk_wnd = gtk_widget_get_window(hWnd)) {
        Display *xdsp = GDK_WINDOW_XDISPLAY(gdk_wnd);
        Window xwnd = GDK_WINDOW_XID(gdk_wnd);
        Atom prop = gdk_x11_get_xatom_by_name("_GTK_FRAME_EXTENTS");
        if (gdk_wnd && xdsp && xwnd != None && prop != None) {
            Atom type = 0;
            int format = 0;
            unsigned long nitems = 0, bytes_after = 0;
            unsigned char *data = nullptr;
            if (XGetWindowProperty(xdsp, xwnd, prop, 0, 4, False, XA_CARDINAL, &type, &format, &nitems, &bytes_after, &data) == Success && data) {
                if (nitems == 4) {
                    long *ext = (long*)data;
                    fr.left = ext[0];
                    fr.right = ext[1];
                    fr.top = ext[2];
                    fr.bottom = ext[3];
                }
                XFree(data);
            }
        }
    }
#endif
}

UIWindow::UIWindow(UIWidget *parent, const Rect &rc, BYTE windowFlags) :
    UIAbstractWindow(parent, ObjectType::WindowType, rc),
    m_centralWidget(nullptr),
    m_contentMargins(0,0,0,0),
    m_resAreaWidth(0),
    m_state(-1),
#ifdef __linux
    m_client_area(nullptr),
    m_radius(0),
    m_is_support_round_corners(true),
#endif
    m_borderless(true),
    m_isResizable(true),
    m_isMaximized(false),
    m_scaleChanged(false),
#ifdef _WIN32
    m_min_size(SM_CXMINTRACK, SM_CYMINTRACK),
    m_max_size(SM_CXMAXTRACK, SM_CYMAXTRACK)
#else
    m_min_size(0, 0),
    m_max_size(G_MAXSHORT, G_MAXSHORT)
#endif
{
    m_borderless = windowFlags & Flags::RemoveSystemDecoration;
#ifdef _WIN32
    m_isThemeActive = isThemeActive();
    m_isTaskbarAutoHideOn = isTaskbarAutoHideOn();

    if (m_borderless && ver < WinVer::Win10) {
        LONG style = ::GetWindowLong(m_hWindow, GWL_STYLE) | WS_OVERLAPPEDWINDOW;
        ::SetWindowLong(m_hWindow, GWL_STYLE, style & ~WS_CAPTION);
        if (ver > WinVer::WinXP)
            metrics()->setMetrics(Metrics::BorderWidth, 1);
    }
    m_isMaximized = IsZoomed(m_hWindow);
    m_dpi = GetLogicalDpi(m_hWindow);
    GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);

    if (m_borderless && ver == WinVer::Win10) {
        HDC hdc = GetDC(NULL);
        m_brdWidth = GetSystemMetrics(SM_CXBORDER) * GetDeviceCaps(hdc, LOGPIXELSX)/96;
        ReleaseDC(NULL, hdc);
        m_brdColor = GetBorderColor(true, palette()->color(Palette::Background));
    }

    int x = rc.x;
    UINT flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED;
    if (m_borderless && ver >= WinVer::Win10) {
        Margins fr;
        GetFrameBounds(m_hWindow, fr);
        x += fr.left;
        flags &= ~SWP_NOMOVE;
    }
    SetWindowPos(m_hWindow, NULL, x, rc.y, 0, 0, flags);
#else
    gtk_window_set_type_hint(GTK_WINDOW(m_hWindow), GDK_WINDOW_TYPE_HINT_NORMAL);
    GdkScreen *scr = gtk_widget_get_screen(m_hWindow);
    if (m_borderless) {
        if (gdk_screen_is_composited(scr)) {
            int radius;
            m_corners = cornersPlacementAndRadius(radius);
            m_radius = radius;
            metrics()->setMetrics(Metrics::BorderRadius, radius);
            GtkWidget *header = gtk_header_bar_new();
            gtk_window_set_titlebar(GTK_WINDOW(m_hWindow), header);
            gtk_widget_destroy(header);
            m_resAreaWidth = WINDOW_THIN_BORDER_WIDTH;
        } else {
            m_is_support_round_corners = false;
            gtk_window_set_decorated(GTK_WINDOW(m_hWindow), FALSE);
            m_resAreaWidth = WINDOW_BORDER_WIDTH;
        }
    } else {
        m_is_support_round_corners = false;
    }

    char style[256];
    snprintf(style, sizeof(style), "decoration {border-radius: %dpx %dpx %dpx %dpx;}", m_radius, m_radius,
             m_corners == UIDrawingEngine::CornerAll ? m_radius : 0, m_corners == UIDrawingEngine::CornerAll ? m_radius : 0);
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, style, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(m_hWindow);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    m_client_area = gtk_bin_get_child(GTK_BIN(m_hWindow));

    gtk_window_set_default_size(GTK_WINDOW(m_hWindow), rc.width + m_frame.left + m_frame.right, rc.height + m_frame.top + m_frame.bottom);
    gtk_widget_realize(m_hWindow);

    if (UIUtils::desktopEnv() == UIUtils::DesktopEnv::XFCE && UIUtils::windowServer() == UIUtils::WindowServer::X11)
        GetFrameBounds(m_hWindow, m_frame);

    gtk_window_resize(GTK_WINDOW(m_hWindow), rc.width + m_frame.left + m_frame.right, rc.height + m_frame.top + m_frame.bottom);
    gtk_window_move(GTK_WINDOW(m_hWindow), rc.x - m_frame.left, rc.y - m_frame.top);
#endif
}

UIWindow::~UIWindow()
{

}

#ifdef _WIN32
void UIWindow::setGeometry(int x, int y, int w, int h)
{
    if (m_borderless && ver >= WinVer::Win10) {
        Margins fr;
        GetFrameBounds(m_hWindow, fr);
        x += fr.left;
    }
    UIAbstractWindow::setGeometry(x, y, w, h);
}

void UIWindow::move(int x, int y)
{
    if (m_borderless && ver >= WinVer::Win10) {
        Margins fr;
        GetFrameBounds(m_hWindow, fr);
        x += fr.left;
    }
    UIAbstractWindow::move(x, y);
}
#else
BYTE UIWindow::cornersPlacementAndRadius(int &radius)
{
    if (!m_is_support_round_corners) {
        radius = WINDOW_CORNER_RADIUS_DEFAULT;
        return UIDrawingEngine::CornerAll;
    }
    switch (UIUtils::desktopEnv()) {
    case UIUtils::DesktopEnv::GNOME:
        radius = WINDOW_CORNER_RADIUS_GNOME;
        break;

    case UIUtils::DesktopEnv::KDE:
        radius = WINDOW_CORNER_RADIUS_KDE;
        return UIDrawingEngine::CornerLTop | UIDrawingEngine::CornerRTop;

    case UIUtils::DesktopEnv::UNITY:
        radius = WINDOW_CORNER_RADIUS_DEFAULT;
        break;

    case UIUtils::DesktopEnv::CINNAMON:
        radius = WINDOW_CORNER_RADIUS_CINNAMON;
        return UIDrawingEngine::CornerLTop | UIDrawingEngine::CornerRTop;

    case UIUtils::DesktopEnv::XFCE:
        radius = WINDOW_CORNER_RADIUS_XFCE;
        return UIDrawingEngine::CornerLTop | UIDrawingEngine::CornerRTop;

    default:
        radius = WINDOW_CORNER_RADIUS_DEFAULT;
        break;
    }
    return UIDrawingEngine::CornerAll;
}

Size UIWindow::size() const
{
    int w = 0, h = 0;
    gtk_window_get_size(GTK_WINDOW(m_hWindow), &w, &h);
    if (!gtk_window_is_maximized(GTK_WINDOW(m_hWindow))) {
        w -= m_frame.left + m_frame.right;
        h -= m_frame.top + m_frame.bottom;
    }
    return Size(w, h);
}

void UIWindow::size(int *width, int *height) const
{
    gtk_window_get_size(GTK_WINDOW(m_hWindow), width, height);
    if (!gtk_window_is_maximized(GTK_WINDOW(m_hWindow))) {
        *width -= m_frame.left + m_frame.right;
        *height -= m_frame.top + m_frame.bottom;
    }
}
#endif

void UIWindow::setMinimumSize(int w, int h)
{
    m_min_size = Size(w, h);
#ifdef _WIN32
    int width = 0, height = 0;
    size(&width, &height);
    SetWindowPos(m_hWindow, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#else
    GdkGeometry hints = {};
    hints.min_width = w;
    hints.min_height = h;
    hints.max_width = m_max_size.width;
    hints.max_height = m_max_size.height;
    gtk_window_set_geometry_hints(GTK_WINDOW(m_hWindow), GTK_WIDGET(m_hWindow), &hints, GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
#endif
}

void UIWindow::setMaximumSize(int w, int h)
{
    m_max_size = Size(w, h);
#ifdef _WIN32
    int width = 0, height = 0;
    size(&width, &height);
    SetWindowPos(m_hWindow, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#else
    GdkGeometry hints = {};
    hints.min_width = m_min_size.width;
    hints.min_height = m_min_size.height;
    hints.max_width = w;
    hints.max_height = h;
    gtk_window_set_geometry_hints(GTK_WINDOW(m_hWindow), GTK_WIDGET(m_hWindow), &hints, GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
#endif
}

void UIWindow::setCentralWidget(UIWidget *wgt)
{
    m_centralWidget = wgt;
}

void UIWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    m_contentMargins = Margins(left, top, right, bottom);
#ifdef _WIN32
    if (IsWindowVisible(m_hWindow))
        SetWindowPos(m_hWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#else
    gtk_widget_queue_resize(m_hWindow); // gtk_widget_queue_allocate(m_hWindow);
#endif
}

void UIWindow::setResizable(bool isResizable)
{
    if (m_isResizable != isResizable) {
        m_isResizable = isResizable;
#ifdef _WIN32
        m_size = size();
        LONG style = ::GetWindowLong(m_hWindow, GWL_STYLE);
        ::SetWindowLong(m_hWindow, GWL_STYLE, m_isResizable ? style | WS_MAXIMIZEBOX : style & ~WS_MAXIMIZEBOX);
#else
        gtk_window_set_resizable(GTK_WINDOW(m_hWindow), isResizable);
#endif
    }
}

void UIWindow::showNormal()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_RESTORE);
#else
    if (gtk_window_is_maximized(GTK_WINDOW(m_hWindow)))
        gtk_window_unmaximize(GTK_WINDOW(m_hWindow));
    gtk_window_present(GTK_WINDOW(m_hWindow));
#endif
}

void UIWindow::showMinimized()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_SHOWMINIMIZED);
#else
    gtk_window_iconify(GTK_WINDOW(m_hWindow));
#endif
}

void UIWindow::showMaximized()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_SHOWMAXIMIZED);
#else
    gtk_window_maximize(GTK_WINDOW(m_hWindow));
#endif
}

#ifdef _WIN32
void UIWindow::setIcon(int id)
{
    HMODULE hInstance = GetModuleHandle(NULL);
    HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(id), IMAGE_ICON, 96, 96, LR_DEFAULTCOLOR | LR_SHARED);
    SendMessage(m_hWindow, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(m_hWindow, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}
#else
void UIWindow::setIcon(const tstring &path)
{
    if (GdkPixbuf *pb = gdk_pixbuf_new_from_resource_at_scale(path.c_str(), 96, 96, TRUE, NULL)) {
        gtk_window_set_icon(GTK_WINDOW(m_hWindow), pb);
        g_object_unref(pb);
    }
}
#endif

bool UIWindow::isMinimized()
{
#ifdef _WIN32
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(wpl);
    if (GetWindowPlacement(m_hWindow, &wpl))
        return wpl.showCmd == SW_SHOWMINIMIZED;
    return false;
#else
    return m_state & GDK_WINDOW_STATE_ICONIFIED;
#endif
}

bool UIWindow::isMaximized()
{
#ifdef _WIN32
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(wpl);
    if (GetWindowPlacement(m_hWindow, &wpl))
        return wpl.showCmd == SW_SHOWMAXIMIZED;
    return false;
#else
    return gtk_window_is_maximized(GTK_WINDOW(m_hWindow));
#endif
}

UIWidget *UIWindow::centralWidget()
{
    return m_centralWidget;
}

int UIWindow::onStateChanged(const FnVoidInt &callback)
{
    m_state_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

void UIWindow::disconnect(int connectionId)
{
    auto it = m_state_callbacks.find(connectionId);
    if (it != m_state_callbacks.end())
        m_state_callbacks.erase(it);
}

#ifdef _WIN32
bool UIWindow::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_DPICHANGED: {
        m_dpi = (double)HIWORD(wParam)/96;
        m_dpi_ratio = m_dpi;
        GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);
        bool isMaximized = m_isMaximized;
        if (isMaximized)
            showNormal();
        NotifyParams params;
        params.wParam = HIWORD(wParam);
        EnumChildWindows(m_hWindow, DpiChangedNotifyProc, (LPARAM)&params);
        if (isMaximized) {
            showMaximized();
        } else {
            RECT *prc = (RECT*)lParam;
            SetWindowPos(m_hWindow, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }
        break;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_CHILD_BUTTONDOWN_NOTIFY: {
        NotifyParams params;
        params.senderHwnd = m_hWindow;
        params.wParam = wParam;
        EnumThreadWindows(GetCurrentThreadId(), ToplevelButtondownNotifyProc, (LPARAM)&params);
        break;
    }

    case WM_KEYDOWN:
    case WM_CHILD_KEYDOWN_NOTIFY: {
        NotifyParams params;
        params.senderHwnd = m_hWindow;
        params.wParam = wParam;
        EnumThreadWindows(GetCurrentThreadId(), ToplevelKeydownNotifyProc, (LPARAM)&params);
        break;
    }

    case WM_PAINT: {
        UIDrawingEngine *de = engine();
        RECT rc;
        GetClientRect(m_hWindow, &rc);
        de->Begin(this, m_hWindow, &rc);
        de->DrawFlatRect();
        if (m_brdWidth != 0)
            de->DrawTopBorder(m_brdWidth, m_brdColor);
        de->End();
        *result = FALSE;
        return true;
    }

    case WM_NCPAINT: {
        if (ver > WinVer::Win7 || !m_borderless)
            return false;
        if (HDC hdc = ::GetDCEx(m_hWindow, 0, DCX_WINDOW | DCX_USESTYLE)) {
            RECT rcc, rcw;
            ::GetClientRect(m_hWindow, &rcc);
            ::GetWindowRect(m_hWindow, &rcw);
            POINT pt;
            pt.x = rcw.left;
            pt.y = rcw.top;
            ::MapWindowPoints(0, m_hWindow, (LPPOINT)&rcw, (sizeof(RECT)/sizeof(POINT)));
            ::OffsetRect(&rcc, -rcw.left, -rcw.top);
            ::OffsetRect(&rcw, -rcw.left, -rcw.top);
            HRGN rgntemp = NULL;
            if (wParam == NULLREGION || wParam == ERROR) {
                ::ExcludeClipRect(hdc, rcc.left, rcc.top, rcc.right, rcc.bottom);
            } else {
                rgntemp = ::CreateRectRgn(rcc.left + pt.x, rcc.top + pt.y, rcc.right + pt.x, rcc.bottom + pt.y);
                if (::CombineRgn(rgntemp, (HRGN)wParam, rgntemp, RGN_DIFF) == NULLREGION) {
                    // nothing to paint
                }
                ::OffsetRgn(rgntemp, -pt.x, -pt.y);
                ::ExtSelectClipRgn(hdc, rgntemp, RGN_AND);
            }
            HBRUSH hbrushBkg = ::CreateSolidBrush(palette()->color(Palette::Background));
            ::FillRect(hdc, &rcw, hbrushBkg);
            ::DeleteObject(hbrushBkg);

            // HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
            // GetWindowRgn(msg->hwnd, hrgn);
            HBRUSH hbrushBrd = ::CreateSolidBrush(palette()->color(Palette::Border));
            ::FrameRect(hdc, &rcw, hbrushBrd); // Drawing NC border when using ~WS_CAPTION
            // ::FrameRgn(hdc, hrgn, hbrushBrd, 1, 1); // Drawing NC border when using WS_CAPTION
            ::DeleteObject(hbrushBrd);
            // ::DeleteObject(hrgn);

            ::ReleaseDC(m_hWindow, hdc);
            if (rgntemp != 0)
                ::DeleteObject(rgntemp);
            return true;
        }
        return false;
    }

    case WM_NCCALCSIZE: {
        if (!m_borderless || !wParam)
            return false;
        NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS*)lParam;
        if (!m_isThemeActive) {
            *result = m_isMaximized ? 0 : DefWindowProc(m_hWindow, WM_NCCALCSIZE, wParam, lParam);
            return true;
        }
        LRESULT res = DefWindowProc(m_hWindow, WM_NCCALCSIZE, wParam, lParam);
        params->rgrc[0].left -= m_frame.left;
        params->rgrc[0].top -= m_frame.top;
        params->rgrc[0].right += m_frame.left;
        params->rgrc[0].bottom += m_frame.left;
        if (m_isMaximized && m_isTaskbarAutoHideOn && (ver >= WinVer::Win10))
            params->rgrc[0].bottom -= 2;
        *result = res;
        return true;
    }

    case WM_NCHITTEST: {
        if (m_isResizable) {
            if (m_borderless) {
                RECT rect;
                GetWindowRect(m_hWindow, &rect);
                long x = GET_X_LPARAM(lParam);
                long y = GET_Y_LPARAM(lParam);
                if (x <= rect.left + m_resAreaWidth) {
                    if (y <= rect.top + m_resAreaWidth)
                        *result = HTTOPLEFT;
                    else
                    if (y > rect.top + m_resAreaWidth && y < rect.bottom - m_resAreaWidth)
                        *result = HTLEFT;
                    else
                    if (y >= rect.bottom - m_resAreaWidth)
                        *result = HTBOTTOMLEFT;
                } else
                if (x > rect.left + m_resAreaWidth && x < rect.right - m_resAreaWidth) {
                    if (y <= rect.top + m_resAreaWidth)
                        *result = HTTOP;
                    else
                    if (y >= rect.bottom - m_resAreaWidth)
                        *result = HTBOTTOM;
                } else
                if (x >= rect.right - m_resAreaWidth) {
                    if (y <= rect.top + m_resAreaWidth)
                        *result = HTTOPRIGHT;
                    else
                    if (y > rect.top + m_resAreaWidth && y < rect.bottom - m_resAreaWidth)
                        *result = HTRIGHT;
                    else
                    if (y >= rect.bottom - m_resAreaWidth)
                        *result = HTBOTTOMRIGHT;
                }
                return *result != 0;
            }
        } else {
            LRESULT hit = DefWindowProc(m_hWindow, msg, wParam, lParam);
            if (hit == HTBOTTOM || hit == HTLEFT || hit == HTRIGHT || hit == HTTOP ||
                    hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT || hit == HTTOPLEFT || hit == HTTOPRIGHT) {
                *result = HTCLIENT;
                return true;
            }
        }
        return false;
    }

    case WM_NCACTIVATE: {
        NotifyParams params;
        params.senderHwnd = m_hWindow;
        params.wParam = wParam;
        EnumChildWindows(m_hWindow, NcActivationNotifyProc, (LPARAM)&params);
        if (m_borderless) {
            if (ver > WinVer::WinXP && ver < WinVer::Win10) {
                // Prevent drawing of inactive system frame (needs ~WS_CAPTION or temporary ~WS_VISIBLE to work)
                *result = DefWindowProc(m_hWindow, WM_NCACTIVATE, wParam, -1);
                return true;
            } else
            if (ver == WinVer::Win10) {
                m_brdColor = GetBorderColor(LOWORD(wParam), palette()->color(Palette::Background));
                RECT rc;
                GetClientRect(m_hWindow, &rc);
                rc.bottom = m_brdWidth;
                RedrawWindow(m_hWindow, &rc, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT | RDW_UPDATENOW);
            }
        }
        EnumThreadWindows(GetCurrentThreadId(), ToplevelNcActivationNotifyProc, (LPARAM)&params);
        return false;
    }

    case WM_GETMINMAXINFO: {
        bool isMaximized = (bool)IsZoomed(m_hWindow);
        if (m_isMaximized != isMaximized) {
            m_isMaximized = isMaximized;
            GetFrameMetricsForDpi(m_frame, m_dpi, isMaximized);
            if (m_borderless && ver == WinVer::Win10) {
                if (isMaximized) {
                    m_brdWidth = 0;
                } else {
                    HDC hdc = GetDC(NULL);
                    m_brdWidth = GetSystemMetrics(SM_CXBORDER) * GetDeviceCaps(hdc, LOGPIXELSX)/96;
                    ReleaseDC(NULL, hdc);
                }
            }
        }

        MINMAXINFO* minMaxInfo = (MINMAXINFO*)lParam;
        if (m_isResizable) {
            minMaxInfo->ptMinTrackSize.x = m_min_size.width;
            minMaxInfo->ptMinTrackSize.y = m_min_size.height;
            if (m_max_size.width != SM_CXMAXTRACK)
                minMaxInfo->ptMaxTrackSize.x = m_max_size.width;
            if (m_max_size.height != SM_CYMAXTRACK)
                minMaxInfo->ptMaxTrackSize.y = m_max_size.height;
        } else {
            minMaxInfo->ptMinTrackSize.x = m_size.width;
            minMaxInfo->ptMinTrackSize.y = m_size.height;
            minMaxInfo->ptMaxTrackSize.x = m_size.width;
            minMaxInfo->ptMaxTrackSize.y = m_size.height;
        }
        break;
    }

    case WM_THEMECHANGED: {
        bool _isThemeActive = isThemeActive();
        if (m_isThemeActive != _isThemeActive)
            m_isThemeActive = _isThemeActive;
        break;
    }

    case WM_SETTINGCHANGE: {
        if (wParam == SPI_SETWINARRANGING) {
            if (ver > UIUtils::WinVer::Win10) {
                NotifyParams params;
                params.wParam = wParam;
                EnumChildWindows(m_hWindow, SettingsChangeNotifyProc, (LPARAM)&params);
            }
        }
        break;
    }

    case WM_SIZE: {            
        switch (wParam) {
        case SIZE_MAXIMIZED:
        case SIZE_MINIMIZED:
        case SIZE_RESTORED: {
            if (m_state != (int)wParam) {
                m_state = (int)wParam;
                for (auto it = m_state_callbacks.begin(); it != m_state_callbacks.end(); it++)
                    if (it->second)
                        (it->second)(m_state);

                if (m_borderless) {
                    if (m_isMaximized) {
                        if (ver < WinVer::Win10) {
                            m_resAreaWidth = 0;
                            Rect rc = availableGeometry(m_hWindow);
                            int offset = 0;
                            if (ver == WinVer::WinXP) {
                                if (isTaskbarAutoHideOn())
                                    offset += NC_AREA_WIDTH + 1;
                                if (m_isThemeActive) {
                                    rc.x += -NC_AREA_WIDTH;
                                    rc.y += -NC_AREA_WIDTH;
                                    rc.width += 2*NC_AREA_WIDTH;
                                    rc.height += 2*NC_AREA_WIDTH;
                                }
                            } else
                            if (ver > WinVer::WinXP && isTaskbarAutoHideOn())
                                offset += 2;
                            UIThread::invoke(this, [=]() {
                                SetWindowPos(m_hWindow, NULL, rc.x, rc.y, rc.width, rc.height - offset, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
                            });
                        }
                    } else {
                        if (ver < WinVer::Win10) {
                            m_resAreaWidth = (int)round(WINDOW_BORDER_WIDTH * m_dpi);
                            if (ver == WinVer::WinXP)
                                m_resAreaWidth -= NC_AREA_WIDTH;
                        }
                    }
                }
            }
            if (m_centralWidget) {
                int top_offset = 0;
                if (m_borderless && !m_isMaximized && ver == UIUtils::WinVer::Win10)
                    top_offset = m_brdWidth;
                m_centralWidget->setGeometry(m_contentMargins.left + m_resAreaWidth, m_contentMargins.top + top_offset + m_resAreaWidth,
                                             LOWORD(lParam) - m_contentMargins.right - m_contentMargins.left - 2*m_resAreaWidth,
                                             HIWORD(lParam) - m_contentMargins.bottom - m_contentMargins.top - top_offset - 2*m_resAreaWidth);
            }
            break;
        }
        default:
            break;
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractWindow::event(msg, wParam, lParam, result);
}
#else
bool UIWindow::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_HOOKED_DRAW: {
        UIDrawingEngine *de = engine();
        int x = 0, y = 0, w = 0, h = 0;
        w = gtk_widget_get_allocated_width(m_client_area);
        h = gtk_widget_get_allocated_height(m_client_area);
        gtk_widget_translate_coordinates(m_client_area, m_hWindow, 0, 0, &x, &y);
        Rect rc(x, y, w, h);
        if (m_is_support_round_corners)
            metrics()->setMetrics(Metrics::BorderRadius, m_isMaximized ? 0 : m_radius);

        de->Begin(this, (cairo_t*)param, &rc);
        de->DrawRoundedRect(m_corners);
        de->End();
        return false;
    }

    case GDK_HOOKED_MAP_AFTER: {
        gtk_widget_queue_resize(m_hWindow); // gtk_widget_queue_allocate(m_hWindow);
        return false;
    }

    case GDK_HOOKED_SIZE_ALLOC: {
        int w = 0, h = 0;
        gtk_window_get_size(GTK_WINDOW(m_hWindow), &w, &h);
        if (!gtk_window_is_maximized(GTK_WINDOW(m_hWindow))) {
            w -= m_frame.left + m_frame.right;
            h -= m_frame.top + m_frame.bottom;
        }

        if (m_centralWidget) {
            m_centralWidget->setGeometry(m_contentMargins.left + m_resAreaWidth, m_contentMargins.top + m_resAreaWidth,
                                         w - m_contentMargins.right - m_contentMargins.left - 2*m_resAreaWidth,
                                         h - m_contentMargins.bottom - m_contentMargins.top - 2*m_resAreaWidth);
        }
        break;
    }

    case GDK_HOOKED_CONFIGURE: {
        return false;
    }

    case GDK_HOOKED_CONFIGURE_AFTER: {
        GdkEventConfigure *cev = (GdkEventConfigure*)param;
        if (!m_isMaximized) {
            // TODO: Detect normal geometry for XFCE
            m_normalPos = Point(cev->x, cev->y);
            m_normalSize = Size(cev->width, cev->height);
        }
        return false;
    }

    case GDK_HOOKED_FOCUS_CHANGE_AFTER: {
        return false;
    }

    case GDK_HOOKED_WINDOW_STATE_AFTER: {
        GdkEvent *ev = (GdkEvent*)param;
        if (ev->window_state.changed_mask & GDK_WINDOW_STATE_FOCUSED) {
            bool focused = ev->window_state.new_window_state & GDK_WINDOW_STATE_FOCUSED;
            if (m_gtk_layout) {
                ParentNotifyProc(m_gtk_layout, GDK_PARENT_ACTIVATION_NOTIFY, (void*)&focused);
            }
            ToplevelNotifyProc(m_hWindow, GDK_TOPLEVEL_ACTIVATION_NOTIFY, (void*)&focused);
        }
        if (ev->window_state.changed_mask & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN)) {
            int m_state = (int)ev->window_state.new_window_state & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN);
            m_isMaximized = m_state & GDK_WINDOW_STATE_MAXIMIZED;
            for (auto it = m_state_callbacks.begin(); it != m_state_callbacks.end(); it++)
                if (it->second)
                    (it->second)(m_state);

            if (m_borderless)
                m_resAreaWidth =  m_isMaximized ? 0 : m_is_support_round_corners ? WINDOW_THIN_BORDER_WIDTH : WINDOW_BORDER_WIDTH;

            // HACK: Exiting from maximized state by dragging the title bar requires an additional resize update for XFCE.
            if (m_borderless && !m_isMaximized && UIUtils::desktopEnv() == UIUtils::DesktopEnv::XFCE)
                g_idle_add_full(G_PRIORITY_LOW, onUpdateGeometry, m_hWindow, NULL);
        }
        return false;
    }

    case GDK_BUTTON_PRESS:
    case GDK_CHILD_BUTTONDOWN_NOTIFY: {
        GdkEventButton *bev = (GdkEventButton*)param;
        ToplevelNotifyProc(m_hWindow, GDK_TOPLEVEL_BUTTONDOWN_NOTIFY, (void*)&bev->button);
        return false;
    }

    case GDK_KEY_PRESS: {
        ToplevelNotifyProc(m_hWindow, GDK_TOPLEVEL_KEYDONW_NOTIFY, (void*)param);
        break;
    }

    default:
        break;
    }
    return UIAbstractWindow::event(ev_type, param);
}
#endif
