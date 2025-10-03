#include "uilabel.h"
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

void UILabel::setText(const tstring &text, bool multiline)
{
    m_text = text;
    m_multiline = multiline;
    update();
}

void UILabel::onPaint(const RECT &rc)
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
    if (!m_text.empty())
        de->DrawString(rc, m_text, m_hFont, m_multiline);
}
