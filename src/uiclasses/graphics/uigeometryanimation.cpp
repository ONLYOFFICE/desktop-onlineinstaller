#include "uigeometryanimation.h"
#include "uiwidget.h"
#include "uitimer.h"
#ifdef _WIN32
# define ANIMATION_INTERVAL USER_TIMER_MINIMUM
#else
# define ANIMATION_INTERVAL 16
#endif


UIGeometryAnimation::UIGeometryAnimation(UIWidget* target) :
    m_target(target),
    m_animationTimer(new UITimer),
    m_currentType(AnimationType::Move),
    m_animationDuration(0)
{
    m_animationTimer->onTimeout([this] {
        onAnimationStep();
    });
}

UIGeometryAnimation::~UIGeometryAnimation()
{
    stopAnimation();
    delete m_animationTimer; m_animationTimer = nullptr;
}

void UIGeometryAnimation::animateTo(const Rect& targetRc, int durationMs, AnimationType type)
{
    if (durationMs <= 0) {
        m_target->setGeometry(targetRc.x, targetRc.y, targetRc.width, targetRc.height);
        return;
    }

    stopAnimation();

    Point pos = m_target->pos();
    Size sz = m_target->size();

    m_startRc = Rect(pos.x, pos.y, sz.width, sz.height);
    m_targetRc = targetRc;
    m_currentType = type;
    m_animationDuration = durationMs;
    m_animationStartTime = std::chrono::steady_clock::now();

    m_animationTimer->start(ANIMATION_INTERVAL, UITimer::Mode::Repeating);
}

void UIGeometryAnimation::stopAnimation()
{
    m_animationTimer->stop();
}

bool UIGeometryAnimation::isAnimating() const noexcept
{
    return m_animationTimer->isActive();
}

void UIGeometryAnimation::onAnimationStep()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_animationStartTime).count();

    if (elapsed >= m_animationDuration) {
        m_target->setGeometry(m_targetRc.x, m_targetRc.y, m_targetRc.width, m_targetRc.height);
        stopAnimation();
        return;
    }

    float progress = static_cast<float>(elapsed) / m_animationDuration;
    progress = progress * progress * (3.0f - 2.0f * progress);

    Rect currentRc = m_startRc;

    if (m_currentType == AnimationType::Move || m_currentType == AnimationType::MoveAndResize) {
        currentRc.x = m_startRc.x + static_cast<int>((m_targetRc.x - m_startRc.x) * progress);
        currentRc.y = m_startRc.y + static_cast<int>((m_targetRc.y - m_startRc.y) * progress);
    }

    if (m_currentType == AnimationType::Resize || m_currentType == AnimationType::MoveAndResize) {
        currentRc.width = m_startRc.width + static_cast<int>((m_targetRc.width - m_startRc.width) * progress);
        currentRc.height = m_startRc.height + static_cast<int>((m_targetRc.height - m_startRc.height) * progress);
    }

    m_target->setGeometry(currentRc.x, currentRc.y, currentRc.width, currentRc.height);
#ifdef __linux__
    m_target->updateGeometry();
#endif
}
