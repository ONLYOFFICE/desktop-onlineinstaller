#include "uilabel.h"
#include "uimetrics.h"
#include "uidrawningengine.h"


UILabel::UILabel(UIWidget *parent) :
    UIWidget(parent, ObjectType::WidgetType),
    UIconHandler(this),
    m_multiline(false)
{

}

UILabel::~UILabel()
{

}

void UILabel::setText(const std::wstring &text, bool multiline)
{
    m_text = text;
    m_multiline = multiline;
    update();
}

bool UILabel::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        //    DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (m_hBmp)
            engine()->DrawImage(m_hBmp);
        if (m_hIcon)
            engine()->DrawIcon(m_hIcon);
        if (m_hEmf)
            engine()->DrawEmfIcon(m_hEmf);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text, m_hFont, m_multiline);

        engine()->End();

        *result = FALSE;
        return true;
    }

    // case WM_MOUSEENTER: {
    //     palette()->setCurrentState(Palette::Hover);
    //     repaint();
    //     break;
    // }

    // case WM_MOUSELEAVE: {
    //     palette()->setCurrentState(Palette::Normal);
    //     repaint();
    //     break;
    // }

    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
