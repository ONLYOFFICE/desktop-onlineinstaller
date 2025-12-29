#include "uilineedit.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include "uiapplication.h"
#include "uiutils.h"
#ifdef __linux
# include "cmath"
# include "widgets/linux/gtkcaret.h"
#endif

#define DEFAULT_CARET_WIDTH 1

using namespace UIUnicode;

UILineEdit::UILineEdit(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this),
    m_sourceText(text),
    m_viewportText(text),
#ifdef _WIN32
    m_caretCreated(false),
#else
    m_caret(nullptr),
#endif
    m_pos(text.length()),
    m_caretPosX(0),
    m_caretPosY(0)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
    metrics()->setMetrics(Metrics::IconAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UILineEdit::~UILineEdit()
{

}

void UILineEdit::setText(const tstring &text) noexcept
{
    UIAbstractButton::setText(text);
    m_sourceText = text;
    m_viewportText = text;
    m_pos = text.length();
    Rect rc;
    textBounds(text, rc);
    m_caretPosX = rc.x + (m_text.empty() || m_pos == 0 ? 0 : rc.width);
    m_caretPosY = rc.y;
#ifdef _WIN32
    if (m_caretCreated)
        SetCaretPos(m_caretPosX, m_caretPosY);
#else
    if (m_caret)
        gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
#endif
}

void UILineEdit::setPlaceholderText(const tstring &text) noexcept
{
    m_placeholderText = text;
    if (m_text.empty() && !m_placeholderText.empty()) {
        m_viewportText = m_placeholderText;
        m_pos = 0;
        Rect rc;
        textBounds(m_placeholderText, rc);
        m_caretPosX = rc.x;
        m_caretPosY = rc.y;
#ifdef _WIN32
        if (m_caretCreated)
            SetCaretPos(m_caretPosX, m_caretPosY);
#else
        if (m_caret)
            gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
#endif
    }
}

#ifdef _WIN32
bool UILineEdit::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_NCHITTEST:
        return false;

    case WM_SETCURSOR:
        if (!m_disabled && LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(nullptr, IDC_IBEAM));
            *result = TRUE;
            return true;
        }
        break;

    case WM_SIZE:
    case WM_DPICHANGED_NOTIFY: {
        SetFocus(m_root_hWnd);

        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        bool rtl = m_rtl;
        const Metrics *mtr = metrics();
        int mrg_top = mtr->value(Metrics::TextMarginTop) * m_dpi_ratio;
        int mrg_left = rtl ? mtr->value(Metrics::TextMarginRight) * m_dpi_ratio : mtr->value(Metrics::TextMarginLeft) * m_dpi_ratio;
        int mrg_right = rtl ? mtr->value(Metrics::TextMarginLeft) * m_dpi_ratio : mtr->value(Metrics::TextMarginRight) * m_dpi_ratio;
        m_viewportRc = Gdiplus::RectF(mrg_left, mrg_top, w - mrg_right - mrg_left, h - mtr->value(Metrics::TextMarginBottom) * m_dpi_ratio - mrg_top);
        break;
    }

    case WM_SETFOCUS: {
        Rect rc;
        tstring text = m_text.empty() ? _T(" ") : m_text.substr(0, m_pos == 0 ? 1 : m_pos);
        textBounds(text, rc);
        m_caretPosX = rc.x + (m_text.empty() || m_pos == 0 ? 0 : rc.width);
        m_caretPosY = rc.y;
        CreateCaret(m_hWindow, NULL, DEFAULT_CARET_WIDTH * m_dpi_ratio, rc.height);
        m_caretCreated = true;
        SetCaretPos(m_caretPosX, m_caretPosY);
        ShowCaret(m_hWindow);
        break;
    }

    case WM_KILLFOCUS: {
        if (m_sourceText != m_text)
            m_sourceText = m_text;
        if (m_caretCreated) {
            m_caretCreated = false;
            DestroyCaret();
        }
        break;
    }

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_HOME:
            if (m_pos > 0 && m_text.length() > 0) {
                m_pos = 0;
                size_t len = charLenAt(m_text, m_pos);
                Rect rc;
                textBounds(m_text.substr(0, len), rc);
                m_caretPosX = rc.x;
            }
            break;

        case VK_END:
            if (m_pos < m_text.length() && m_text.length() > 0) {
                m_pos = m_text.length();
                Rect rc;
                textBounds(m_text.substr(0, m_pos), rc);
                m_caretPosX = rc.x + rc.width;
            }
            break;

        case VK_LEFT:
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -= len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos == 0 ? len : m_pos), rc);
                m_caretPosX = rc.x + (m_pos == 0 ? 0 : rc.width);
            }
            break;

        case VK_RIGHT:
            if (m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_pos += len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos), rc);
                m_caretPosX = rc.x + rc.width;
            }
            break;

        case VK_DELETE:
            if (m_text.length() > 0) {
                size_t len = charLenAt(m_text, m_pos);
                m_text.erase(m_pos, len);
                m_viewportText = m_text.empty() && !m_placeholderText.empty() ? m_placeholderText : m_text;
                update();
            }
            break;
        }

        if (m_caretCreated)
            SetCaretPos(m_caretPosX, m_caretPosY);
        break;

    case WM_CHAR:
        switch (wParam) {
        case 0x08: // Backspace
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -= len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos == 0 ? len : m_pos), rc);
                m_caretPosX = rc.x + (m_pos == 0 ? 0 : rc.width);
                SendMessage(m_hWindow, WM_KEYDOWN, VK_DELETE, 1L);
            }
            break;

        case 0x09: // Tab
        case 0x0A: // Linefeed
            break;

        case 0x1B: // Escape
            m_text = m_sourceText;
            m_viewportText = m_text;
            m_pos = m_text.length();
            update();
        case 0x0D: // Return        
            SetFocus(m_root_hWnd);
            break;

        default:
            wchar_t ch = (wchar_t)wParam;
            m_text.insert(m_pos, 1, ch);
            m_viewportText = m_text;
            update();
            m_pos++;
            Rect rc;
            textBounds(m_text.substr(0, m_pos), rc);
            m_caretPosX = rc.x + rc.width;
            break;
        }

        if (m_caretCreated)
            SetCaretPos(m_caretPosX, m_caretPosY);
        break;

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UILineEdit::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_ENTER_NOTIFY: {
        if (!m_disabled) {
            GdkDisplay *display = gtk_widget_get_display(m_hWindow);
            GdkCursor *cursor = gdk_cursor_new_from_name(display, "text");
            GdkWindow *gdk_wnd = gtk_widget_get_window(m_hWindow);
            if (gdk_wnd && cursor) {
                gdk_window_set_cursor(gdk_wnd, cursor);
                g_object_unref(cursor);
            }
        }
        break;
    }

    case GDK_HOOKED_SIZE_ALLOC: {
        GtkAllocation *alc = (GtkAllocation*)param;
        const Metrics *mtr = metrics();
        int mrg_top = mtr->value(Metrics::TextMarginTop) * m_dpi_ratio;
        int mrg_left = (m_rtl ? mtr->value(Metrics::TextMarginRight) : mtr->value(Metrics::TextMarginLeft)) * m_dpi_ratio;
        int mrg_right = (m_rtl ? mtr->value(Metrics::TextMarginLeft) : mtr->value(Metrics::TextMarginRight)) * m_dpi_ratio;
        int mrg_bottom = mtr->value(Metrics::TextMarginBottom) * m_dpi_ratio;
        m_viewportRc = Rect(mrg_left, mrg_top, alc->width - mrg_right - mrg_left, alc->height - mrg_bottom - mrg_top);
        break;
    }

    case GDK_FOCUS_CHANGE: {
        GdkEventFocus *fev = (GdkEventFocus*)param;
        if (fev->in == 1) {
            Rect rc;
            tstring text = m_text.empty() ? _T(" ") : m_text.substr(0, m_pos == 0 ? charLenAt(m_text, m_pos) : m_pos);
            textBounds(text, rc);
            m_caretPosX = rc.x + (m_text.empty() || m_pos == 0 ? 0 : rc.width);
            m_caretPosY = rc.y;
            if (!m_caret) {
                m_caret = gtk_caret_create(m_hWindow, DEFAULT_CARET_WIDTH * m_dpi_ratio, rc.height);
                gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
                gtk_caret_show(GTK_CARET(m_caret));
            }
        } else {
            if (m_sourceText != m_text)
                m_sourceText = m_text;
            if (m_caret) {
                gtk_caret_destroy(GTK_CARET(m_caret));
                m_caret = nullptr;
            }
        }
        break;
    }

    case GDK_KEY_PRESS: {
        GdkEventKey *kev = (GdkEventKey*)param;
        switch (kev->keyval) {
        case GDK_KEY_Home:
            if (m_pos > 0 && m_text.length() > 0) {
                m_pos = 0;
                size_t len = charLenAt(m_text, m_pos);
                Rect rc;
                textBounds(m_text.substr(0, len), rc);
                m_caretPosX = rc.x;
            }
            break;

        case GDK_KEY_End:
            if (m_pos < m_text.length() && m_text.length() > 0) {
                m_pos = m_text.length();
                Rect rc;
                textBounds(m_text.substr(0, m_pos), rc);
                m_caretPosX = rc.x + rc.width;
            }
            break;

        case GDK_KEY_Left:
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -= len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos == 0 ? len : m_pos), rc);
                m_caretPosX = rc.x + (m_pos == 0 ? 0 : rc.width);
            }
            break;

        case GDK_KEY_Right:
            if (m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_pos += len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos), rc);
                m_caretPosX = rc.x + rc.width;
            }
            break;

        case GDK_KEY_Delete:
            if (m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_text.erase(m_pos, len);
                m_viewportText = m_text;
                update();
            }
            break;

        case GDK_KEY_BackSpace:
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -=len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos == 0 ? len : m_pos), rc);
                m_caretPosX = rc.x + (m_pos == 0 ? 0 : rc.width);
                if (m_pos < m_text.length()) {
                    size_t len = charLenAt(m_text, m_pos);
                    m_text.erase(m_pos, len);
                    m_viewportText = m_text;
                    update();
                }
            }
            break;

        case GDK_KEY_Tab:
        case GDK_KEY_Linefeed:
            break;

        case GDK_KEY_Escape:
            m_text = m_sourceText;
            m_viewportText = m_text;
            m_pos = m_text.length();
            update();
        case GDK_KEY_Return:
            if (GtkWidget *parent = gtk_widget_get_parent(m_hWindow))
                gtk_widget_grab_focus(parent);
            break;

        default:
            gunichar uni = gdk_keyval_to_unicode(kev->keyval);
            if (uni != 0) {
                char utf8[5] = {0};
                int len = g_unichar_to_utf8(uni, utf8);
                m_text.insert(m_pos, utf8, len);
                m_viewportText = m_text;
                update();
                m_pos += len;
                Rect rc;
                textBounds(m_text.substr(0, m_pos), rc);
                m_caretPosX = rc.x + rc.width;
            }
            break;
        }

        if (m_caret)
            gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
        return true;
    }

    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UILineEdit::onPaint(const RECT &rc)
{
    UIDrawingEngine *de = engine();
#ifdef _WIN32
    if (m_hBmp)
        de->DrawImage(m_hBmp);
    if (m_hIcon)
        de->DrawIcon(m_hIcon);
    if (m_hEmf)
        de->DrawEmfIcon(m_hEmf);
#else
    if (m_hBmp)
        de->DrawIcon(m_hBmp);
    if (m_hSvg)
        de->DrawSvgIcon(m_hSvg);
#endif
    if (!m_viewportText.empty())
        de->DrawString(rc, m_viewportText, m_hFont);
}

void UILineEdit::textBounds(const tstring &text, Rect &rc)
{
#ifdef _WIN32    
    HDC hdc = GetDC(m_hWindow);
    LOGFONTW logFont = {0};
    GetObject(m_hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(hdc, &logFont);

    Gdiplus::Graphics gr(hdc);
    gr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::CharacterRange range(0, text.length());
    Gdiplus::StringAlignment h_algn, v_algn;
    UINT algn = metrics()->value(Metrics::TextAlignment);
    if (algn & Metrics::AlignHLeft)
        h_algn = Gdiplus::StringAlignmentNear;
    if (algn & Metrics::AlignHCenter)
        h_algn = Gdiplus::StringAlignmentCenter;
    if (algn & Metrics::AlignHRight)
        h_algn = Gdiplus::StringAlignmentFar;
    if (algn & Metrics::AlignVTop)
        v_algn = Gdiplus::StringAlignmentNear;
    if (algn & Metrics::AlignVCenter)
        v_algn = Gdiplus::StringAlignmentCenter;
    if (algn & Metrics::AlignVBottom)
        v_algn = Gdiplus::StringAlignmentFar;
    Gdiplus::StringFormat strFmt(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
    strFmt.SetAlignment(h_algn);
    strFmt.SetLineAlignment(v_algn);
    strFmt.SetMeasurableCharacterRanges(1, &range);
    if (m_rtl)
        strFmt.SetFormatFlags(Gdiplus::StringFormatFlagsDirectionRightToLeft);

    Gdiplus::Region rgn;
    gr.MeasureCharacterRanges(text.c_str(), text.length(), &font, m_viewportRc, &strFmt, 1, &rgn);

    Gdiplus::RectF bounds;
    rgn.GetBounds(&bounds, &gr);
    rc.x = bounds.X;
    rc.y = bounds.Y;
    rc.width = round(bounds.Width);
    rc.height = round(bounds.Height);
    ReleaseDC(m_hWindow, hdc);
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(m_hWindow, text.c_str());
    pango_layout_set_font_description(lut, m_hFont);
    pango_layout_set_wrap(lut, PANGO_WRAP_WORD);
    int txt_w, txt_h;
    pango_layout_get_size(lut, &txt_w, &txt_h);
    txt_w /= PANGO_SCALE;
    txt_h /= PANGO_SCALE;

    int text_x = m_viewportRc.x;
    int text_y = m_viewportRc.y;
    PangoAlignment h_algn = PANGO_ALIGN_LEFT;
    int algn = metrics()->value(Metrics::TextAlignment);
    if (algn & Metrics::AlignHLeft) {
        h_algn = PANGO_ALIGN_LEFT;
    }
    if (algn & Metrics::AlignHCenter) {
        h_algn = PANGO_ALIGN_CENTER;
        // if (!multiline)
            text_x += round((m_viewportRc.width - txt_w) / 2.0);
    }
    if (algn & Metrics::AlignHRight) {
        h_algn = PANGO_ALIGN_RIGHT;
        // if (!multiline)
            text_x += m_viewportRc.width - txt_w;
    }
    if (algn & Metrics::AlignVTop)
        text_y += 0;
    if (algn & Metrics::AlignVCenter)
        text_y += round((m_viewportRc.height - txt_h) / 2.0);
    if (algn & Metrics::AlignVBottom)
        text_y += m_viewportRc.height - txt_h;

    pango_layout_set_alignment(lut, h_algn);
    g_object_unref(lut);

    rc.x = text_x;
    rc.y = text_y;
    rc.width = txt_w;
    rc.height = txt_h;
#endif
}
