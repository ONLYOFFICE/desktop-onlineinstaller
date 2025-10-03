#ifndef LAYOUT_H
#define LAYOUT_H

#include "uiobject.h"
#include "uilayoutitem.h"
#include "uicommon.h"
// #include <vector>



class UILayout : public UILayoutItem
{
public:
    UILayout(UIObject *parent = nullptr);
    virtual ~UILayout();

    virtual void addWidget(UIWidget *wgt) = 0;

protected:
    Margins m_margins;

private:
    friend class UIWidget;
    virtual void onResize(int w, int h) = 0;
};

#endif // LAYOUT_H
