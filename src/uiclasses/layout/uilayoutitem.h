#ifndef UILAYOUTITEM_H
#define UILAYOUTITEM_H

#include "uidefines.h"


class UIWidget;
// class UILayout;
class UISpacer;
class DECL_VISUALUI UILayoutItem
{
public:
    UILayoutItem();
    ~UILayoutItem();

    UIWidget *widget() const noexcept;
    // UILayout *layout() const noexcept;
    UISpacer *spacer() const noexcept;

private:
    friend class UIBoxLayout;
    void calcSize(double dpi);
    void move(int x, int y);
    void setGeometry(int x, int y, int w, int h);

    UIWidget *wgt;
    // UILayout *lut;
    UISpacer *spr;
    int hsb,
        vsb,
        width,
        height;
};

#endif // UILAYOUTITEM_H
