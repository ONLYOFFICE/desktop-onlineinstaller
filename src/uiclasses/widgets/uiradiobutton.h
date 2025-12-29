#ifndef UIRADIOBUTTON_H
#define UIRADIOBUTTON_H

#include "uiabstractbutton.h"


class DECL_VISUALUI UIRadioButton : public UIAbstractButton
{
public:
    explicit UIRadioButton(UIWidget *parent = nullptr, const tstring &text = {});
    ~UIRadioButton();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onPaint(const RECT &rc) override;
    virtual void click() override;
};

#endif // UIRADIOBUTTON_H
