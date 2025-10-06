#ifndef UIMETRICS_H
#define UIMETRICS_H

#include "uidefines.h"

class DECL_VISUALUI Metrics
{
public:
    Metrics();
    ~Metrics();

    enum Alignment : unsigned char {
        AlignNone    = 0,
        AlignHLeft   = 1,
        AlignHCenter = 2,
        AlignHRight  = 4,
        AlignVTop    = 8,
        AlignVCenter = 16,
        AlignVBottom = 32,
        AlignCenter  = AlignHCenter | AlignVCenter
    };

    enum Role : unsigned char {
        BorderWidth,
        BorderRadius,
        IconWidth,
        IconHeight,
        IconMarginLeft,
        IconMarginTop,
        IconMarginRight,
        IconMarginBottom,
        IconAlignment,
        PrimitiveWidth,
        AlternatePrimitiveWidth,
        PrimitiveRadius,
        ShadowWidth,
        ShadowRadius,
        TextMarginLeft,
        TextMarginTop,
        TextMarginRight,
        TextMarginBottom,
        TextAlignment,
        METRICS_LAST
    };

    void setMetrics(Role, int) noexcept;
    int  value(Role) const noexcept;

private:
    int metrics[METRICS_LAST];
};

#endif // UIMETRICS_H
