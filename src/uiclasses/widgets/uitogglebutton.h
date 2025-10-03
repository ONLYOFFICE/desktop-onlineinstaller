#ifndef UITOGGLEBUTTON_H
#define UITOGGLEBUTTON_H

#include "uiabstractbutton.h"


class DECL_VISUALUI UIToggleButton : public UIAbstractButton
{
public:
    explicit UIToggleButton(UIWidget *parent = nullptr, const tstring &text = {});
    ~UIToggleButton();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
    virtual void onPaint(const RECT &rc) override;
    virtual void click() override;    
};

#endif // UITOGGLEBUTTON_H
