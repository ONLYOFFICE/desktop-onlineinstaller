#ifndef UILAYOUT_H
#define UILAYOUT_H

#include "uidefines.h"
#include "uilayoutitem.h"
#include "uicommon.h"


class UIWidget;
class DECL_VISUALUI UILayout
{
public:
    enum Alignment : unsigned char {
        AlignHLeft   = 1,
        AlignHCenter = 2,
        AlignHRight  = 4,
        AlignVTop    = 8,
        AlignVCenter = 16,
        AlignVBottom = 32,
        AlignCenter  = AlignHCenter | AlignVCenter
    };

    explicit UILayout(int alignment = AlignHLeft | AlignVTop);
    virtual ~UILayout();

    virtual void addWidget(UIWidget *wgt) = 0;
    virtual void addSpacer(UISpacer *spr) = 0;

protected:
    std::vector<UILayoutItem> m_items;
    Margins m_margins;
    int m_alignment,
        m_spacing,
        m_width,
        m_height;

private:
    friend class UIWidget;
    friend class UIDialog;
    friend class UIAbstractPopup;
    virtual void onResize(int w, int h, double dpi = 1.0) noexcept = 0;
};

#endif // UILAYOUT_H
