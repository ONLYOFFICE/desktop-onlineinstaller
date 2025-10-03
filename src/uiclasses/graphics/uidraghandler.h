#ifndef UIDRAGHANDLER_H
#define UIDRAGHANDLER_H

#include "uidefines.h"
#include "uiplatformtypes.h"
#ifdef __linux__
# include "uicommon.h"
#endif


class UIWidget;
class DECL_VISUALUI UIDragHandler
{
public:
    explicit UIDragHandler(UIWidget *target);
    ~UIDragHandler();

    void handleButtonDownEvent(int x, int y);
    void handleButtonUpEvent();
    void handleMouseMoveEvent(int x, int y);

private:
    UIWidget *m_target;
    PlatformWindow m_hWindow;
#ifdef __linux__
    GtkWidget *m_parent;
#endif
    POINT m_dragStart;
    POINT m_winStart;
    bool m_dragging;
};

#endif // UIDRAGHANDLER_H
