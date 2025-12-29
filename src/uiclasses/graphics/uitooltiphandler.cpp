#include "uitooltiphandler.h"
#include "uifontmetrics.h"
#include "uitooltip.h"
#include "uiwidget.h"
#include "uicursor.h"
#include "uitimer.h"

UIToolTipHandler::UIToolTipHandler(UIWidget* parent) :
    m_parent(parent),
    m_tooltip(nullptr),
    m_checkTimer(new UITimer),
    m_lastCursorPos(Point(-1, -1)),
    m_tooltipTickCounter(0),
    m_tooltipSet(false)
{
    m_checkTimer->onTimeout([this] {
        onToolTipCheck();
    });
}

UIToolTipHandler::~UIToolTipHandler()
{
    hideToolTip();
    delete m_checkTimer; m_checkTimer = nullptr;
}

void UIToolTipHandler::setToolTipText(const tstring &text) noexcept
{
    m_tooltipText = text;
    m_tooltipSet = !text.empty();
}

void UIToolTipHandler::handleMouseMove()
{
    if (m_tooltipSet && !m_checkTimer->isActive()) {
#ifdef _WIN32
        int x = 0, y = 0;
        UICursor::globalPos(x, y);
        if (x != m_lastCursorPos.x || y != m_lastCursorPos.y)
            m_checkTimer->start(1000);
#else
        m_checkTimer->start(1000);
#endif
    }
}

void UIToolTipHandler::skipToolTip()
{
    if (!m_tooltip && m_checkTimer->isActive())
        m_checkTimer->stop();
}

void UIToolTipHandler::hideToolTip()
{
    m_checkTimer->stop();
    if (m_tooltip) {
        m_tooltip->close();
        m_tooltip = nullptr;
    }
    m_tooltipTickCounter = 0;
}

void UIToolTipHandler::onToolTipCheck()
{    
    if (!m_parent->underMouse()) {
        hideToolTip();
        return;
    }

    if (m_tooltip) {
        if (m_tooltipTickCounter > 2) {
            hideToolTip();
            return;
        }
    } else
    if (!m_tooltipText.empty()) {
        int x = 0, y = 0;
        int width = 0, height = 0;
        UIFontMetrics fm;
        fm.textSize(m_parent, m_parent->m_hFont, m_tooltipText, width, height);
        double dpi = m_parent->dpiRatio();
        width += 20 * dpi;
        height += 10 * dpi;
        UICursor::globalPos(x, y);
        m_lastCursorPos = Point(x, y);
        m_tooltip = new UIToolTip(m_parent->topLevelWidget(), Rect(x + 10 * dpi, y + 10 * dpi, width, height));
        m_tooltip->setObjectGroupId(_T("ToolTip"));
        m_tooltip->setText(m_tooltipText);
        m_tooltip->show();
        m_tooltip->onClose([=](bool *accept) {
            *accept = true;
            if (m_checkTimer->isActive())
                m_checkTimer->stop();
            m_tooltipTickCounter = 0;
            m_tooltip = nullptr;
        });
    }
    ++m_tooltipTickCounter;
}
