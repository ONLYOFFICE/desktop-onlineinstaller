/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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
