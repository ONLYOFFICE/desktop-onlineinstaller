#include "uilayoutitem.h"
#include "uiwidget.h"
#include "uispacer.h"

UILayoutItem::UILayoutItem() :
    wgt(nullptr),
    // lut(nullptr),
    spr(nullptr),
    hsb(0),
    vsb(0),
    width(0),
    height(0)
{

}

UILayoutItem::~UILayoutItem()
{

}

UIWidget *UILayoutItem::widget() const noexcept
{
    return wgt;
}

// UILayout *UILayoutItem::layout() const noexcept
// {
//     return lut;
// }

UISpacer *UILayoutItem::spacer() const noexcept
{
    return spr;
}

void UILayoutItem::calcSize(double dpi)
{
    if (wgt) {
        wgt->size(&width, &height);
    } else
    if (spr) {
        width = spr->width*dpi;
        height = spr->height*dpi;
    }
}

void UILayoutItem::move(int x, int y)
{
    if (wgt) {
        wgt->move(x, y);
    } else
    if (spr) {
        spr->x = x;
        spr->y = y;
    }
}

void UILayoutItem::setGeometry(int x, int y, int w, int h)
{
    if (wgt) {
        wgt->setGeometry(x, y, w, h);
    } else
    if (spr) {
        spr->x = x;
        spr->y = y;
        spr->width = w;
        spr->height = h;
    }
}
