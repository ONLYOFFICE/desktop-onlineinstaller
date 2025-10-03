#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "uiobject.h"
#include "uiplatformtypes.h"
#include "uicommon.h"
#include "uidrawingsurface.h"
#include <unordered_map>
#ifndef CW_USEDEFAULT
# define CW_USEDEFAULT 100
#endif


class UILayout;
class UIDragHandler;
class UIGeometryAnimation;
class DECL_VISUALUI UIWidget : public UIObject, public UIDrawningSurface
{
public:
    explicit UIWidget(UIWidget *parent = nullptr);
    virtual ~UIWidget();

    virtual void setObjectGroupId(const tstring &id) override;
    virtual void setGeometry(int, int, int, int);
    virtual void move(int, int);
    virtual void resize(int, int);
    void setDisabled(bool);
    virtual void close();
    UIWidget* parentWidget() const noexcept;
    virtual Size size() const;
    virtual void size(int*, int*) const;
    virtual Point pos() const;
    void updateGeometry();
    UIGeometryAnimation* geometryAnimation();
    void setGeometryAnimation(UIGeometryAnimation*);
    UIDragHandler* dragHandler();
    void setDragHandler(UIDragHandler*);
    void applyStyle();
    void setSizePolicy(SizePolicy::Properties, int);
    void setFont(const tstring &font, double fontPointSize = 10);
    void setBaseSize(int w, int h);
    void setCorners(unsigned char corner);
    void setAcceptDrops(bool);
    void show();
    void hide();
    void repaint();
    void update();
    void bringAboveSiblings();
    void setLayout(UILayout *lut);
    bool isCreated();
    bool isActive();
    bool underMouse();
    void grabMouse();
    void ungrabMouse();
    int  sizePolicy(SizePolicy::Properties);
    double dpiRatio();
    UILayout* layout() const noexcept;
    PlatformWindow platformWindow() const noexcept;
    UIWidget* topLevelWidget() const noexcept;
    static UIWidget* widgetFromHwnd(UIWidget *parent, PlatformWindow);

    /* callback */
    int onResize(const FnVoidIntInt &callback);
    int onMove(const FnVoidIntInt &callback);
    int onAboutToDestroy(const FnVoidVoid &callback);
    int onCreate(const FnVoidVoid &callback);
    int onActivationChanged(const FnVoidBool &callback);
    int onClose(const FnVoidBoolPtr &callback);
    int onDropFiles(const FnVoidVecStr &callback);
    int onContextMenu(const FnVoidIntInt &callback);

    virtual void onInvokeMethod(long long wParam);
    virtual void disconnect(int) override;

protected:
    friend class UIApplication;
    friend class UIToolTipHandler;
    UIWidget(UIWidget *parent, ObjectType type, PlatformWindow hWindow = nullptr, const Rect &rc = Rect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT));
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*);
    LRESULT checkInputRegion(LPARAM lParam, const RECT &rc);
#else
    virtual bool event(uint ev_type, void *param);
    void updateInputRegion(const RECT &rc);
#endif
    virtual void onPaint(const RECT &rc);
    virtual void onAfterCreated();

    PlatformFont   m_hFont;
    PlatformWindow m_hWindow;
#ifdef _WIN32
    HWND m_root_hWnd;
    bool m_root_is_layered;
#endif
    UILayout    *m_layout;
    double       m_dpi_ratio;
    unsigned char m_corners;
    bool         m_disabled,
                 m_rtl;

private:
    friend class UICheckBox;
    friend class UIRadioButton;
    friend class UIToggleButton;

#ifdef __linux
    virtual GtkWidget* gtkLayout();
    Size m_size;
    Point m_pos;
#endif
    void setPlatformWindow(PlatformWindow);

    int m_size_behaviors[SizePolicy::PROPERTIES_LAST];
    std::unordered_map<int, FnVoidBool>   m_activation_callbacks;
    std::unordered_map<int, FnVoidIntInt> m_resize_callbacks,
                                          m_move_callbacks,
                                          m_context_callbacks;
    std::unordered_map<int, FnVoidVoid>   m_create_callbacks,
                                          m_destroy_callbacks;
    std::unordered_map<int, FnVoidBoolPtr> m_close_callbacks;
    std::unordered_map<int, FnVoidVecStr>  m_drop_callbacks;

    Size    m_base_size;
    tstring m_font;
    UIDragHandler *m_drag_handler;
    UIGeometryAnimation *m_geometry_animation;
    double m_font_size;
    bool   m_is_created,
           m_is_active,
           m_is_destroyed,
           m_is_class_destroyed,
           m_mouse_entered;
};

#endif // UIWIDGET_H
