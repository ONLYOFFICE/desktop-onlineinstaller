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

struct DECL_VISUALUI FontInfo {
    FontInfo(std::string _name = DEFAULT_FONT_NAME, float _pointSize = 10.0f, int _weight = 400,
             bool _italic = false, bool _underline = false, bool _strikeOut = false) :
        name(_name),
        pointSize(_pointSize),
        weight(_weight),
        italic(_italic),
        underline(_underline),
        strikeOut(_strikeOut)
    {}

    std::string name;
    float pointSize;
    int weight;
    bool italic;
    bool underline;
    bool strikeOut;
};

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
