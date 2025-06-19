#include "label.h"
#include "metrics.h"
#include "drawningengine.h"


Label::Label(Widget *parent) :
    Widget(parent, ObjectType::WidgetType),
    IconHandler(this),
    m_multiline(false)
{

}

Label::~Label()
{

}

void Label::setText(const std::wstring &text, bool multiline)
{
    m_text = text;
    m_multiline = multiline;
    update();
}

bool Label::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
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
    return Widget::event(msg, wParam, lParam, result);
}
