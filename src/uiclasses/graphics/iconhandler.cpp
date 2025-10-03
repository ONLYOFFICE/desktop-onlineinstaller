#include "iconhandler.h"
#include "metrics.h"
#include "widget.h"
#include "baseutils.h"


IconHandler::IconHandler(Widget *owner) :
    m_hIcon(nullptr),
    m_hEmf(nullptr),
    m_hBmp(nullptr),
    m_owner(owner)
{

}

IconHandler::~IconHandler()
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    if (m_hEmf) {
        delete m_hEmf, m_hEmf = nullptr;
    }
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
}

void IconHandler::setIcon(const std::wstring &path, int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    m_hIcon = (HICON)LoadImage(NULL, path.c_str(), IMAGE_ICON, w, h, LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_SHARED);
    m_owner->update();
}

void IconHandler::setIcon(int id, int w, int h)
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    m_hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, w, h, LR_COPYFROMRESOURCE | LR_DEFAULTCOLOR | LR_SHARED);
    m_owner->update();
}

void IconHandler::setEMFIcon(const std::wstring &path, int w, int h)
{
    if (m_hEmf) {
        delete m_hEmf, m_hEmf = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_hEmf = new Gdiplus::Metafile(path.c_str());
    m_owner->update();
}

void IconHandler::setEMFIcon(int id, int w, int h)
{
    if (m_hEmf) {
        delete m_hEmf, m_hEmf = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    Utils::loadEmfResource(m_hEmf, id, RT_RCDATA);
    m_owner->update();
}

void IconHandler::setImage(int id, int w, int h)
{
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    Utils::loadImageResource(m_hBmp, id, L"PNG");
    m_owner->update();
}

void IconHandler::setImage(const std::wstring &path, int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_hBmp = new Gdiplus::Bitmap(path.c_str());
    m_owner->update();
}

void IconHandler::setIconSize(int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_owner->update();
}
