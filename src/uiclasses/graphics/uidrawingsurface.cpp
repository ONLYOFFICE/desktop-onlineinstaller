#include "uidrawingsurface.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include "uipalette.h"


UIDrawningSurface::UIDrawningSurface() :
    m_metrics(new Metrics),
    m_palette(new Palette)
{
    m_engine = UIDrawingEngine::instance();
}

UIDrawningSurface::~UIDrawningSurface()
{
    delete m_palette, m_palette = nullptr;
    delete m_metrics, m_metrics = nullptr;
}

Metrics *UIDrawningSurface::metrics() const noexcept
{
    return m_metrics;
}

Palette *UIDrawningSurface::palette() const noexcept
{
    return m_palette;
}

UIDrawingEngine *UIDrawningSurface::engine() const noexcept
{
    return m_engine;
}
