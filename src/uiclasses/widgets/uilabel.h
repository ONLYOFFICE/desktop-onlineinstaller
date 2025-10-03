#ifndef UILABEL_H
#define UILABEL_H

#include "uiwidget.h"
#include "uiconhandler.h"


class DECL_VISUALUI UILabel : public UIWidget, public UIconHandler
{
public:
    explicit UILabel(UIWidget *parent = nullptr);
    virtual ~UILabel();

    void setText(const tstring &text, bool multiline = false);

    /* callback */

protected:
    virtual void onPaint(const RECT &rc) override;
    tstring m_text;

private:
    bool  m_multiline;
};

#endif // UILABEL_H
