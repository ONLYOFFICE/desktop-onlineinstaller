#ifndef UIGEOMETRYANIMATION_H
#define UIGEOMETRYANIMATION_H

#include "uidefines.h"
#include "uicommon.h"
#include <chrono>


class UIWidget;
class UITimer;
class DECL_VISUALUI UIGeometryAnimation
{
public:
    enum class AnimationType {
        Move,
        Resize,
        MoveAndResize
    };

    explicit UIGeometryAnimation(UIWidget* target);
    ~UIGeometryAnimation();

    void animateTo(const Rect &targetRc, int durationMs, AnimationType type = AnimationType::MoveAndResize);
    void stopAnimation();
    bool isAnimating() const noexcept;

private:
    void onAnimationStep();

    std::chrono::time_point<std::chrono::steady_clock> m_animationStartTime;
    UIWidget *m_target;
    UITimer  *m_animationTimer;
    Rect m_startRc;
    Rect m_targetRc;
    AnimationType m_currentType;
    int m_animationDuration;
};

#endif // UIGEOMETRYANIMATION_H
