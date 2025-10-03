#ifndef UILINEEDIT_H
#define UILINEEDIT_H

#include "uiabstractbutton.h"
#include "uiconhandler.h"


class DECL_VISUALUI UILineEdit : public UIAbstractButton, public UIconHandler
{
public:
    explicit UILineEdit(UIWidget *parent = nullptr, const tstring &text = {});
    ~UILineEdit();

    virtual void setText(const tstring &text) noexcept override;
    void setPlaceholderText(const tstring &text) noexcept;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onPaint(const RECT &rc) override;

private:
    void textBounds(const tstring &text, Rect &rc);

    tstring m_sourceText,
            m_placeholderText,
            m_viewportText;
#ifdef _WIN32
    bool m_caretCreated;
    Gdiplus::RectF m_viewportRc;
#else
    GtkWidget *m_caret;
    Rect m_viewportRc;
#endif
    uint32_t m_pos,
         m_caretPosX,
         m_caretPosY;
};

#endif // UILINEEDIT_H
