#ifndef UIPOPUPMESSAGE_H
#define UIPOPUPMESSAGE_H

#include "uiabstractpopup.h"

class UILabel;
class UIBoxLayout;
class UIEventLoop;
class DECL_VISUALUI UIPopupMessage : public UIAbstractPopup
{
public:
    explicit UIPopupMessage(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_TOOL_RECT);
    ~UIPopupMessage();

    enum ButtonRole : BYTE {
        Ok,
        Yes,
        No,
        Apply,
        Retry,
        Close,
        Cancel,
        Continue,
        Reset,
        Help,
        Open,
        Save,
        None
    };

    void setCaptionText(const tstring &text);
    void setDescriptionText(const tstring &text);
    void addButton(const tstring &text, ButtonRole role);
    int runDialog();

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif

private:
    UILabel *m_labelCaption,
            *m_labelDescription;
    UIWidget *buttonsWidget;
    UIBoxLayout *m_buttonsLayout;
    UIEventLoop *m_loop;
    int m_selected;
};

#endif // UIPOPUPMESSAGE_H
