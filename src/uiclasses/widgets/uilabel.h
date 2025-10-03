#ifndef LABEL_H
#define LABEL_H

#include "uiwidget.h"
#include "uiconhandler.h"


class UILabel : public UIWidget, public UIconHandler
{
public:
    UILabel(UIWidget *parent = nullptr);
    virtual ~UILabel();

    void setText(const std::wstring &text, bool multiline = false);
    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    std::wstring m_text;
    bool  m_multiline;
};

#endif // LABEL_H
