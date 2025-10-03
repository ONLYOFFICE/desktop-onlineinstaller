#ifndef LAYOUT_H
#define LAYOUT_H

#include "uiobject.h"
#include "uilayoutitem.h"
#include "uicommon.h"
// #include <vector>



class Layout : public LayoutItem
{
public:
    Layout(Object *parent = nullptr);
    virtual ~Layout();

    virtual void addWidget(Widget *wgt) = 0;

protected:
    Margins m_margins;

private:
    friend class Widget;
    virtual void onResize(int w, int h) = 0;
};

#endif // LAYOUT_H
