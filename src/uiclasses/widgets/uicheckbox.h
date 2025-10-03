#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "uiabstractbutton.h"


class UICheckBox : public UIAbstractButton
{
public:
    UICheckBox(UIWidget *parent = nullptr, const std::wstring &text = L"");
    virtual ~UICheckBox();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void click() override;

private:
    bool m_checked;
};

#endif // CHECKBOX_H
