#ifndef BOXLAYOUT_H
#define BOXLAYOUT_H

#include "uilayout.h"
#include <unordered_map>


class UIBoxLayout : public UILayout
{
public:
    enum Direction : unsigned char {
        Horizontal,
        Vertical
    };
    UIBoxLayout(Direction);
    ~UIBoxLayout();

    virtual void addWidget(UIWidget *wgt) override;
    virtual void setContentMargins(int, int, int, int);
    virtual void setSpacing(int);

protected:

private:
    virtual void onResize(int w, int h) override;
    std::unordered_map<UIWidget*, int> m_destroy_conn;
    std::vector<UIWidget*> m_widgets;
    Direction m_direction;
    int m_spacing;
    int m_total_fixed_size;
};

#endif // BOXLAYOUT_H
