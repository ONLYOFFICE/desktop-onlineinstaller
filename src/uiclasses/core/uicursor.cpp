#include "uicursor.h"
#ifdef _WIN32
# include <Windows.h>
#else
# include <gtk/gtk.h>
#endif


UICursor::UICursor()
{

}

UICursor::~UICursor()
{

}

void UICursor::globalPos(int &x, int &y)
{
#ifdef _WIN32
    POINT pt = {0,0};
    GetCursorPos(&pt);
    x = pt.x;
    y = pt.y;
#else
    GdkDisplay *display = gdk_display_get_default();
    GdkDeviceManager *device_manager = gdk_display_get_device_manager(display);
    GdkDevice *device = gdk_device_manager_get_client_pointer(device_manager);
    gdk_device_get_position (device, NULL, &x, &y);
#endif
}
