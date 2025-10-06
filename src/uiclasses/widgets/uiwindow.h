#ifndef UIWINDOW_H
#define UIWINDOW_H

#include "uiabstractwindow.h"

#define DEFAULT_WINDOW_RECT Rect(100,100,1368,768)


class DECL_VISUALUI UIWindow : public UIAbstractWindow
{
public:
    explicit UIWindow(UIWidget *parent = nullptr, const Rect &rc = DEFAULT_WINDOW_RECT, BYTE windowFlags = 0);
    virtual ~UIWindow();

    enum Flags : BYTE {
        RemoveSystemDecoration = 1
    };

    enum State {
#ifdef _WIN32
        Normal = SIZE_RESTORED,
        Minimized = SIZE_MINIMIZED,
        Maximized = SIZE_MAXIMIZED
#else
        Normal = 0,
        Minimized = GDK_WINDOW_STATE_ICONIFIED,
        Maximized = GDK_WINDOW_STATE_MAXIMIZED
#endif
    };

#ifdef _WIN32
    virtual void setGeometry(int, int, int, int) override;
    virtual void move(int, int) override;
#else
    BYTE cornersPlacementAndRadius(int &radius);
    virtual Size size() const override;
    virtual void size(int*, int*) const override;
#endif
    void setMinimumSize(int w, int h);
    void setMaximumSize(int w, int h);
    void setCentralWidget(UIWidget*);
    void setContentsMargins(int, int, int, int);
    void setResizable(bool);
    void showNormal();
    void showMinimized();
    void showMaximized();
#ifdef _WIN32
    void setIcon(int);
#else
    void setIcon(const tstring&);
#endif
    void setLayout(UILayout*) = delete;
    bool isMinimized();
    bool isMaximized();
    UIWidget *centralWidget();
    UILayout *layout() = delete;

    /* callback */
    int onStateChanged(const FnVoidInt &callback);

    virtual void disconnect(int) override;

protected:    
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif

private:
    UIWidget *m_centralWidget;
    Margins  m_contentMargins;
    COLORREF m_brdColor;
    int      m_brdWidth,
             m_resAreaWidth,
             m_state;
    Margins  m_frame;
#ifdef _WIN32
    double   m_dpi;
#else
    GtkWidget *m_client_area;
    int      m_radius;
    bool     m_is_support_round_corners;
#endif
    bool     m_borderless,
             m_isResizable,
             m_isMaximized,
#ifdef _WIN32
             m_isThemeActive,
             m_isTaskbarAutoHideOn,
#endif
             m_scaleChanged;

    Size     m_min_size,
             m_max_size;
#ifdef _WIN32
    Size     m_size;
#endif
    std::unordered_map<int, FnVoidInt> m_state_callbacks;
};

#endif // WINDOW_H
