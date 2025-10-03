#ifndef UIABSTRACTWINDOW_H
#define UIABSTRACTWINDOW_H

#include "uiwidget.h"

#define DEFAULT_RECT Rect(100,100,800,600)


class DECL_VISUALUI UIAbstractWindow : public UIWidget
{
public:
    explicit UIAbstractWindow(UIWidget *parent = nullptr, ObjectType type = ObjectType::WindowType, const Rect &rc = DEFAULT_RECT);
    virtual ~UIAbstractWindow();

    void setWindowTitle(const tstring &title);
#ifdef __linux__
    virtual void setGeometry(int, int, int, int) override;
    virtual void move(int, int) override;
    virtual void resize(int, int) override;
#endif
    virtual Size size() const override;
    virtual void size(int*, int*) const override;
    virtual Point pos() const override;
    void showAll();

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
    virtual GtkWidget* gtkLayout() override;

    GtkWidget *m_gtk_layout;
#endif
    Point m_normalPos;
    Size m_normalSize;

private:
    tstring m_title;
};

#endif // UIABSTRACTWINDOW_H
