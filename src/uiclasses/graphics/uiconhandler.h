#ifndef UICONHANDLER_H
#define UICONHANDLER_H

#include "uidefines.h"
#include "uipixmap.h"
#ifdef _WIN32
# define setVectorIcon(id, w, h) setEMFIcon(id, w, h)
#else
# define setVectorIcon(id, w, h) setSVGIcon(id, w, h)
#endif

class UIWidget;
struct _RsvgHandle;
class DECL_VISUALUI UIconHandler
{
public:
    explicit UIconHandler(UIWidget *owner);
    virtual ~UIconHandler();

    void setIcon(const tstring &path, int w, int h);
#ifdef _WIN32
    void setIcon(int id, int w, int h);
    void setEMFIcon(const tstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setImage(int id, int w, int h);
#else
    void setIcon(const char *id, int w, int h);
    void setSVGIcon(const tstring &path, int w, int h);
    void setSVGIcon(const char *id, int w, int h);
    void setImage(const char *id, int w, int h);
#endif
    void setImage(const tstring &path, int w, int h);
    void setPixmap(const UIPixmap &pixmap);
    void setPixmap(UIPixmap &&pixmap);
    void setIconSize(int w, int h);

protected:
    PlatformBitmap *m_hBmp;
#ifdef _WIN32
    HICON m_hIcon;
    Gdiplus::Metafile *m_hEmf;
#else
    _RsvgHandle *m_hSvg;
#endif

private:
    UIWidget *m_owner;
};

#endif // UICONHANDLER_H
