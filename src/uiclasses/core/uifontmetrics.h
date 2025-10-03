#ifndef UIFONTMETRICS_H
#define UIFONTMETRICS_H

#include "uidefines.h"
#include "uiplatformtypes.h"


class UIWidget;
class DECL_VISUALUI UIFontMetrics
{
public:
    UIFontMetrics();
    ~UIFontMetrics();

    void textSize(UIWidget *wgt, PlatformFont hFont, const tstring &text, int &width, int &height) const;
};

#endif // UIFONTMETRICS_H
