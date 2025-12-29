#include "uibutton.h"
#include "uidrawningengine.h"
#ifdef _WIN32
# include <windowsx.h>
# include "uipalette.h"
# include "uiutils.h"

static bool isArrangingAllowed() {
    BOOL arranging = FALSE;
    SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
    return (arranging == TRUE);
}
#else
#endif


UIButton::UIButton(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this),
    m_stockIcon(StockIcon::None)
#ifdef _WIN32
    , supportSnapLayouts(false),
    snapLayoutAllowed(false),
    snapLayoutTimerIsSet(false)
#else
#endif
{

}

UIButton::~UIButton()
{

}

void UIButton::setSupportSnapLayouts()
{
#ifdef _WIN32
    if (UIUtils::winVersion() > UIUtils::WinVer::Win10) {
        snapLayoutAllowed = isArrangingAllowed();
        supportSnapLayouts = true;
    }
#endif
}

void UIButton::setStockIcon(StockIcon stockIcon)
{
    m_stockIcon = stockIcon;
    update();
}

#ifdef _WIN32
bool UIButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_NCHITTEST: {
        if (supportSnapLayouts && snapLayoutAllowed) {
            if (!snapLayoutTimerIsSet) {
                snapLayoutTimerIsSet = true;
                palette()->setCurrentState(Palette::Hover);
                SetTimer(m_hWindow, SNAP_LAYOUTS_TIMER_ID, 100, NULL);
                repaint();
            }
            *result = HTMAXBUTTON;
            return true;
        }
        return false;
    }

    case WM_TIMER: {
        if (wParam == SNAP_LAYOUTS_TIMER_ID) {
            if (!underMouse()) {
                KillTimer(m_hWindow, wParam);
                snapLayoutTimerIsSet = false;
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case WM_CAPTURECHANGED: {
        if (UIUtils::winVersion() > UIUtils::WinVer::Win10) {
            click();
        }
        break;
    }

    case WM_SETTINGCHANGE_NOTIFY: {
        if (wParam == SPI_SETWINARRANGING)
            snapLayoutAllowed = isArrangingAllowed();
        break;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UIButton::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UIButton::onPaint(const RECT &rc)
{
    UIDrawingEngine *de = engine();
#ifdef _WIN32
    if (m_hIcon)
        de->DrawIcon(m_hIcon);
    if (m_hBmp)
        de->DrawImage(m_hBmp);
    if (m_hEmf)
        de->DrawEmfIcon(m_hEmf);
#else
    if (m_hBmp)
        de->DrawIcon(m_hBmp);
    if (m_hSvg)
        de->DrawSvgIcon(m_hSvg);
#endif
    if (!m_text.empty())
        de->DrawString(rc, m_text, m_hFont);

    if (m_stockIcon == StockIcon::CloseIcon)
        de->DrawStockCloseIcon();
    else
    if (m_stockIcon == StockIcon::RestoreIcon)
        de->DrawStockRestoreIcon();
    else
    if (m_stockIcon == StockIcon::MinimizeIcon)
        de->DrawStockMinimizeIcon();
    else
    if (m_stockIcon == StockIcon::MaximizeIcon)
        de->DrawStockMaximizeIcon();
}
