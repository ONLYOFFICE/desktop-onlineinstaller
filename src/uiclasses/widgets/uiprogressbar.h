#ifndef UIPROGRESSBAR_H
#define UIPROGRESSBAR_H

#include "uiwidget.h"

class UITimer;
class DECL_VISUALUI UIProgressBar : public UIWidget
{
public:
    explicit UIProgressBar(UIWidget *parent = nullptr);
    ~UIProgressBar();

    void setProgress(int progress);
    void pulse(bool);
    void setPulseStep(int);

    /* callback */

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onPaint(const RECT &rc) override;

private:
    UITimer *m_timer;
    int  m_progress,
         m_pulse_pos,
         m_pulse_direction,
         m_pulse_step;
};

#endif // UIPROGRESSBAR_H
