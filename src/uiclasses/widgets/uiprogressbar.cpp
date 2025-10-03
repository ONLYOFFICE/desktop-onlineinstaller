#include "uiprogressbar.h"
#include "uidrawningengine.h"
#include "uitimer.h"
#ifdef __linux__
# include "uiapplication.h"

# define DEFAULT_TIMER_STEP 16
#else
# define DEFAULT_TIMER_STEP USER_TIMER_MINIMUM
#endif
#define DEFAULT_PULSE_STEP 1

UIProgressBar::UIProgressBar(UIWidget *parent) :
    UIWidget(parent, ObjectType::WidgetType),
    m_timer(new UITimer),
    m_progress(0),
    m_pulse_pos(-1),
    m_pulse_direction(1),
    m_pulse_step(DEFAULT_PULSE_STEP)
{
    m_timer->onTimeout([this]() {
        m_pulse_pos += m_pulse_direction * m_pulse_step;
        if (m_pulse_pos >= 100) {
            m_pulse_pos = 100;
            m_pulse_direction = -1;
        } else if (m_pulse_pos <= 0) {
            m_pulse_pos = 0;
            m_pulse_direction = 1;
        }
        update();
    });
}

UIProgressBar::~UIProgressBar()
{
    delete m_timer; m_timer = nullptr;
}

void UIProgressBar::setProgress(int progress)
{
    m_progress = progress;
    update();
}

void UIProgressBar::pulse(bool enable)
{
    m_pulse_pos = enable ? 0 : -1;
    m_pulse_direction = 1;
    if (enable) {
        m_timer->start(DEFAULT_TIMER_STEP);
    } else {
        m_timer->stop();
    }
}

void UIProgressBar::setPulseStep(int step)
{
    if (step < 1)
        step = 1;
    else
    if (step > 50)
        step = 50;
    m_pulse_step = step;
}

#ifdef _WIN32
bool UIProgressBar::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIProgressBar::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}
#endif

void UIProgressBar::onPaint(const RECT&)
{
    engine()->DrawProgressBar(m_progress, m_pulse_pos);
}
