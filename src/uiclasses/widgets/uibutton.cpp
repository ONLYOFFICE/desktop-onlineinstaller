#include "uibutton.h"
#include "uiutils.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include "uipalette.h"
#include <windowsx.h>


static bool isArrangingAllowed() {
    BOOL arranging = FALSE;
    SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
    return (arranging == TRUE);
}

UIButton::UIButton(UIWidget *parent, const std::wstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this),
    m_stockIcon(StockIcon::None),
    supportSnapLayouts(false),
    snapLayoutAllowed(false),
    snapLayoutTimerIsSet(false)
{

}

UIButton::~UIButton()
{

}

void UIButton::setSupportSnapLayouts()
{
    if (UIUtils::getWinVersion() > UIUtils::WinVer::Win10) {
        snapLayoutAllowed = isArrangingAllowed();
        supportSnapLayouts = true;
    }
}

void UIButton::setStockIcon(StockIcon stockIcon)
{
    m_stockIcon = stockIcon;
    update();
}

bool UIButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        // engine()->DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (m_hIcon)
            engine()->DrawIcon(m_hIcon);
        if (m_hEmf)
            engine()->DrawEmfIcon(m_hEmf);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text, m_hFont);

        if (m_stockIcon == StockIcon::CloseIcon)
            engine()->DrawStockCloseIcon();
        else
        if (m_stockIcon == StockIcon::RestoreIcon)
            engine()->DrawStockRestoreIcon();
        else
        if (m_stockIcon == StockIcon::MinimizeIcon)
            engine()->DrawStockMinimizeIcon();
        else
        if (m_stockIcon == StockIcon::MaximizeIcon)
            engine()->DrawStockMaximizeIcon();

        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_NCHITTEST: {
        if (supportSnapLayouts && snapLayoutAllowed) {
            if (!snapLayoutTimerIsSet) {
                snapLayoutTimerIsSet = true;
                palette()->setCurrentState(Palette::Hover);
                SetTimer(m_hWnd, SNAP_LAYOUTS_TIMER_ID, 100, NULL);
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
                KillTimer(m_hWnd, wParam);
                snapLayoutTimerIsSet = false;
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case WM_CAPTURECHANGED: {
        if (UIUtils::getWinVersion() > UIUtils::WinVer::Win10) {
            click();
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
