#ifndef UIOPACITYANIMATION_H
#define UIOPACITYANIMATION_H

#include "uidefines.h"

class UIWidget;
class UIOpacityAnimationPrivate;
class DECL_VISUALUI UIOpacityAnimation
{
public:
    explicit UIOpacityAnimation(UIWidget *target);
    ~UIOpacityAnimation();

    void startFadeIn();
    void startFadeOut();
    void stopAnimation();
    bool isFadingOut() const;
    int opacity() const;

private:
    UIOpacityAnimationPrivate *pimpl;
};

#endif // UIOPACITYANIMATION_H
