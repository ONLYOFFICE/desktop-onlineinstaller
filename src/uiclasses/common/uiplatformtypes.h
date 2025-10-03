#ifndef UIPLATFORMTYPES_H
#define UIPLATFORMTYPES_H

#ifdef _WIN32
# include <Windows.h>
  typedef HWND PlatformWindow;
  typedef HFONT PlatformFont;
#else
# include <cstdint>
# include <gtk/gtk.h>
  typedef unsigned char BYTE;
  typedef uint16_t WORD;
  typedef uint32_t DWORD;
  typedef DWORD COLORREF;
  typedef GtkWidget* PlatformWindow;
  typedef PangoFontDescription* PlatformFont;
#endif

#endif // UIPLATFORMTYPES_H
