#ifndef UISTYLE_H
#define UISTYLE_H

#ifdef _WIN32
# include <Windows.h>
#else
# include <cstdint>
  typedef unsigned char BYTE;
  typedef uint32_t DWORD;
#endif
#include "uidefines.h"


class UIWidget;
class DECL_VISUALUI UIStyle
{
public:
    UIStyle(const UIStyle&) = delete;
    UIStyle& operator=(const UIStyle&) = delete;
    static UIStyle& instance();

    void registerWidget(UIWidget *w);
    void unregisterWidget(UIWidget *w);
#ifdef _WIN32
    void loadThemesFromResource(int id);
    void loadStylesFromResource(int id);
#else
    void loadThemesFromResource(GResource *res, const tstring &id);
    void loadStylesFromResource(GResource *res, const tstring &id);
#endif
    void loadThemesFromFile(const tstring &filePath);
    void loadStylesFromFile(const tstring &filePath);
    void setDefaultTheme(const tstring &theme);
    void setTheme(const tstring &theme);
    void setStyle(UIWidget *w) const;
    DWORD themeColor(const tstring &tag) const;

private:
    UIStyle();
    ~UIStyle();

    class UIStylePrivate;
    UIStylePrivate *pimpl = nullptr;
};

#endif // UISTYLE_H
