#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "uiabstractbutton.h"
#include "uiconhandler.h"


class DECL_VISUALUI UIButton : public UIAbstractButton, public UIconHandler
{
public:
    explicit UIButton(UIWidget *parent = nullptr, const tstring &text = {});
    ~UIButton();

    enum StockIcon : BYTE {
        None,
        MinimizeIcon,
        MaximizeIcon,
        RestoreIcon,
        CloseIcon
    };

    void setSupportSnapLayouts();
    void setStockIcon(StockIcon stockIcon);

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    LRESULT checkInputRegion(LPARAM lParam, const RECT &rc) = delete;
#else
    virtual bool event(uint ev_type, void *param) override;
    void updateInputRegion(const Rect &rc) = delete;
#endif
    virtual void onPaint(const RECT &rc) override;

private:
    int  m_stockIcon;
#ifdef _WIN32
    bool supportSnapLayouts,
         snapLayoutAllowed;
    bool snapLayoutTimerIsSet;
#else
#endif
};

#endif // UIBUTTON_H
