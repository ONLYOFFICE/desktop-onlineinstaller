#include "uicommon.h"


Margins::Margins() :
    left(0), top(0), right(0), bottom(0)
{}

Margins::Margins(int l, int t, int r, int b) :
    left(l), top(t), right(r), bottom(b)
{}


Rect::Rect() :
    x(0), y(0), width(0), height(0)
{}

Rect::Rect(int x, int y, int w, int h) :
    x(x), y(y), width(w), height(h)
{}


Point::Point() :
    x(0), y(0)
{}

Point::Point(int x, int y) :
    x(x), y(y)
{}


Size::Size() :
    width(0), height(0)
{}

Size::Size(int w, int h) :
    width(w), height(h)
{}
