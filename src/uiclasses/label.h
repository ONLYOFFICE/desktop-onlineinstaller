#ifndef LABEL_H
#define LABEL_H

#include "widget.h"
#include "iconhandler.h"


class Label : public Widget, public IconHandler
{
public:
    Label(Widget *parent = nullptr);
    virtual ~Label();

    void setText(const std::wstring &text, bool multiline = false);
    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    std::wstring m_text;
    bool  m_multiline;
};

#endif // LABEL_H
