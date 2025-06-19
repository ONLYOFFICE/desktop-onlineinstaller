#ifndef BUTTON_H
#define BUTTON_H

#include "abstractbutton.h"
#include "iconhandler.h"


class Button : public AbstractButton, public IconHandler
{
public:
    Button(Widget *parent = nullptr, const std::wstring &text = L"");
    virtual ~Button();

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
