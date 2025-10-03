#include "uiconhandler.h"
#include "uimetrics.h"
#include "uiwidget.h"
#include "uiutils.h"


UIconHandler::UIconHandler(UIWidget *owner) :
    m_hIcon(nullptr),
    m_hEmf(nullptr),
    m_hBmp(nullptr),
    m_owner(owner)
{

}

UIconHandler::~UIconHandler()
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

void UIconHandler::setIcon(const std::wstring &path, int w, int h)
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

void UIconHandler::setIcon(int id, int w, int h)
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

void UIconHandler::setEMFIcon(const std::wstring &path, int w, int h)
{
    if (m_hEmf) {
        delete m_hEmf, m_hEmf = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_hEmf = new Gdiplus::Metafile(path.c_str());
    m_owner->update();
}

void UIconHandler::setEMFIcon(int id, int w, int h)
{
    if (m_hEmf) {
        delete m_hEmf, m_hEmf = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    UIUtils::loadEmfResource(m_hEmf, id, RT_RCDATA);
    m_owner->update();
}

void UIconHandler::setImage(int id, int w, int h)
{
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    UIUtils::loadImageResource(m_hBmp, id, L"PNG");
    m_owner->update();
}

void UIconHandler::setImage(const std::wstring &path, int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_hBmp = new Gdiplus::Bitmap(path.c_str());
    m_owner->update();
}

void UIconHandler::setIconSize(int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_owner->update();
}
