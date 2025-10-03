#ifndef ICONHANDLER_H
#define ICONHANDLER_H

#include <Windows.h>
#include <gdiplus.h>
#include <string>


class Widget;
class IconHandler
{
public:
    IconHandler(Widget *owner);
    virtual ~IconHandler();

    void setIcon(const std::wstring &path, int w, int h);
    void setIcon(int id, int w, int h);
    void setEMFIcon(const std::wstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setImage(int id, int w, int h);
    void setImage(const std::wstring &path, int w, int h);
    void setIconSize(int w, int h);

protected:
    HICON m_hIcon;
    Gdiplus::Metafile *m_hEmf;
    Gdiplus::Bitmap *m_hBmp;

private:
    Widget *m_owner;
};

#endif // ICONHANDLER_H
