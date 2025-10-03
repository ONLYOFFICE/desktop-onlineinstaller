#ifndef DRAWNINGSURFACE_H
#define DRAWNINGSURFACE_H


class Metrics;
class Palette;
class UIDrawingEngine;
class UIDrawningSurface
{
public:
    UIDrawningSurface();
    virtual ~UIDrawningSurface();

    Metrics *metrics();
    Palette *palette();

protected:
    UIDrawingEngine *engine();

private:
    Metrics *m_metrics;
    Palette *m_palette;
    UIDrawingEngine *m_engine;
};

#endif // DRAWNINGSURFACE_H
