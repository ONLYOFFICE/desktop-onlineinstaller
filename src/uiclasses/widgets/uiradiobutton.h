#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "uiabstractbutton.h"


class UIRadioButton : public UIAbstractButton
{
public:
    UIRadioButton(UIWidget *parent = nullptr, const std::wstring &text = L"");
    virtual ~UIRadioButton();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void click() override;

private:
    bool m_checked;
};

#endif // RADIOBUTTON_H
