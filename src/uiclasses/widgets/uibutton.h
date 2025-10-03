#ifndef BUTTON_H
#define BUTTON_H

#include "uiabstractbutton.h"
#include "uiconhandler.h"


class UIButton : public UIAbstractButton, public UIconHandler
{
public:
    UIButton(UIWidget *parent = nullptr, const std::wstring &text = L"");
    virtual ~UIButton();

    enum StockIcon : BYTE {
        None,
        MinimizeIcon,
        MaximizeIcon,
        RestoreIcon,
        CloseIcon
    };

    void setSupportSnapLayouts();
    void setStockIcon(StockIcon stockIcon);

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    int  m_stockIcon;
    bool supportSnapLayouts,
         snapLayoutAllowed;
    bool snapLayoutTimerIsSet;
};

#endif // BUTTON_H
