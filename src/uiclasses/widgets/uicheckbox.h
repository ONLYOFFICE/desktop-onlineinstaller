#ifndef UICHECKBOX_H
#define UICHECKBOX_H

#include "uiabstractbutton.h"


class DECL_VISUALUI UICheckBox : public UIAbstractButton
{
public:
    explicit UICheckBox(UIWidget *parent = nullptr, const tstring &text = {});
    ~UICheckBox();

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

#endif // UICHECKBOX_H
