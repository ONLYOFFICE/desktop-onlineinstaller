#ifndef UIDIALOG_H
#define UIDIALOG_H

#include "uiabstractwindow.h"


#define DEFAULT_DLG_RECT Rect(100,100,800,600)

class UIEventLoop;
class DECL_VISUALUI UIDialog : public UIAbstractWindow
{
public:
    explicit UIDialog(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_DLG_RECT);
    virtual ~UIDialog();

    enum DialogCode : BYTE {
        Accepted,
        Rejected
    };

#ifdef __linux__
    virtual void setGeometry(int, int, int, int) override;
    virtual void resize(int, int) override;
#endif
    void accept() noexcept;
    void reject() noexcept;
    int runDialog();

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif

private:
    UIEventLoop *m_loop;
    int m_result;
};

#endif // UIDIALOG_H
