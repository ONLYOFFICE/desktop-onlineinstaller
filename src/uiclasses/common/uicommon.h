#ifndef UICOMMON_H
#define UICOMMON_H

#include "uidefines.h"
#ifdef __linux__
# include "gtk/gtk.h"
#endif

struct DECL_VISUALUI Margins {
    Margins();
    Margins(int, int, int, int);

    int left, top, right, bottom;
};

struct DECL_VISUALUI Rect {
    Rect();
    Rect(int, int, int, int);

    int x, y, width, height;
};

struct DECL_VISUALUI Point {
    Point();
    Point(int, int);

    int x, y;
};

struct DECL_VISUALUI Size {
    Size();
    Size(int, int);

    int width, height;
};

#ifdef __linux__
typedef Point POINT;
typedef Rect RECT;

struct DECL_VISUALUI DropFilesInfo {
    GdkDragContext *context;
    gint x, y;
    GtkSelectionData *sel_data;
    guint info, time;
};
#endif

namespace SizePolicy
{
    enum Properties : unsigned char {
        HSizeBehavior,
        VSizeBehavior,
        PROPERTIES_LAST
    };

    enum SizeBehavior : unsigned char {
        Fixed,
        Expanding,
        SIZE_BEHAVIOR_LAST
    };
}

#endif // COMMON_H
