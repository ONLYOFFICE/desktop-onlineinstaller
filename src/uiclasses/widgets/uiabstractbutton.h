#ifndef UIABSTRACTBUTTON_H
#define UIABSTRACTBUTTON_H

#include "uiwidget.h"
#include <unordered_map>


class UIToolTipHandler;
class DECL_VISUALUI UIAbstractButton : public UIWidget
{
public:
    explicit UIAbstractButton(UIWidget *parent = nullptr, const tstring &text = {});
    virtual ~UIAbstractButton();

    virtual void setText(const tstring &text) noexcept;
    void setToolTip(const tstring &text) noexcept;
    tstring text() noexcept;
    void adjustSizeBasedOnContent();

    /* callback */
    int onClick(const FnVoidVoid &callback);
    virtual void disconnect(int) override;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void click();

    tstring  m_text;
    RECT m_check_rc;
    UIToolTipHandler *m_tooltipHandler;
    bool m_checked;

private:
    std::unordered_map<int, FnVoidVoid> m_click_callbacks;
};

#endif // UIABSTRACTBUTTON_H
