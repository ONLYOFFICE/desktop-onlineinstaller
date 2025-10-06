#include "uiopacityanimation.h"
#include "uiwidget.h"
#include "uitimer.h"

#define OPACITY_TIMER_STEP     16
#define OPACITY_ANIMATION_STEP 25

class UIOpacityAnimationPrivate
{
public:
    UIOpacityAnimationPrivate(UIWidget *target) :
        target(target),
        timer(new UITimer)
    {
        timer->onTimeout([this]() {
            handleAnimationStep();
        });
    }

    ~UIOpacityAnimationPrivate()
    {
        delete timer; timer = nullptr;
    }

    void handleAnimationStep()
    {
        if (fadeOut) {
            opacity = std::max<int>(0, opacity - OPACITY_ANIMATION_STEP);
            if (opacity == 0) {
                timer->stop();
                target->UIWidget::close();
            }
        } else {
            opacity = std::min<int>(255, opacity + OPACITY_ANIMATION_STEP);
            if (opacity == 255) {
                timer->stop();
            }
        }
#ifdef _WIN32
        target->update();
#else
        gtk_widget_set_opacity(target->platformWindow(), (double)opacity/255);
#endif
    }

    UIWidget *target = nullptr;
    UITimer  *timer = nullptr;
    int  opacity = 0;
    bool fadeOut = false;
};

UIOpacityAnimation::UIOpacityAnimation(UIWidget *target) :
    pimpl(new UIOpacityAnimationPrivate(target))
{
#ifdef __linux__
    gtk_widget_set_opacity(target->platformWindow(), 0);
#endif
}

UIOpacityAnimation::~UIOpacityAnimation()
{
    stopAnimation();
    delete pimpl, pimpl = nullptr;
}

void UIOpacityAnimation::startFadeIn()
{
    pimpl->fadeOut = false;
    pimpl->timer->start(OPACITY_TIMER_STEP);
}

void UIOpacityAnimation::startFadeOut()
{
    if (pimpl->fadeOut)
        return;
    pimpl->fadeOut = true;
    pimpl->timer->start(OPACITY_TIMER_STEP);
}

void UIOpacityAnimation::stopAnimation()
{
    pimpl->timer->stop();
}

bool UIOpacityAnimation::isFadingOut() const
{
    return pimpl->fadeOut;
}

int UIOpacityAnimation::opacity() const
{
    return pimpl->opacity;
}
