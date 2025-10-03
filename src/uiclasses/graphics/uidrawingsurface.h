#ifndef UIDRAWNINGSURFACE_H
#define UIDRAWNINGSURFACE_H

#include "uidefines.h"

class Metrics;
class Palette;
class UIDrawingEngine;
class DECL_VISUALUI UIDrawningSurface
{
public:
    UIDrawningSurface();
    virtual ~UIDrawningSurface();

    Metrics *metrics() const noexcept;
    Palette *palette() const noexcept;

protected:
    UIDrawingEngine *engine() const noexcept;

private:
    Metrics *m_metrics;
    Palette *m_palette;
    UIDrawingEngine *m_engine;
};

#endif // UIDRAWNINGSURFACE_H
