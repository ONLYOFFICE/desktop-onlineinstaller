#ifndef UIBOXLAYOUT_H
#define UIBOXLAYOUT_H

#include "uilayout.h"
#include <unordered_map>


class DECL_VISUALUI UIBoxLayout : public UILayout
{
public:
    enum Direction : unsigned char {
        Horizontal,
        Vertical
    };

    explicit UIBoxLayout(Direction, int alignment = AlignHLeft | AlignVTop);
    ~UIBoxLayout();

    virtual void addWidget(UIWidget *wgt) override;
    virtual void addSpacer(UISpacer *spr) override;
    virtual void setContentMargins(int, int, int, int) noexcept;
    virtual void setSpacing(int) noexcept;

private:
    virtual void onResize(int w, int h, double dpi = 1.0) noexcept override;
    std::unordered_map<UIWidget*, int> m_destroy_conn;
    Direction m_direction;
    int m_total_fixed_size;
};

#endif // UIBOXLAYOUT_H
