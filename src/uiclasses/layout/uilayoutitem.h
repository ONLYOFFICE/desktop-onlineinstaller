#ifndef LAYOUTITEM_H
#define LAYOUTITEM_H


class UIWidget;
class UILayout;

class UILayoutItem
{
public:
    UILayoutItem();
    ~UILayoutItem();

    virtual UIWidget *widget();
    virtual UILayout *layout();

protected:
};

#endif // LAYOUTITEM_H
