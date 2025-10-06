#include "gtkcaret.h"

#define DEFAULT_BLINK_INTERVAL 500

struct _GtkCaret {
    GtkDrawingArea parent;
    GtkWidget *parent_wgt;
    gboolean blink;
    guint blink_timeout;
};

typedef struct _GtkCaretClass {
    GtkDrawingAreaClass parent_class;
} GtkCaretClass;

G_DEFINE_TYPE(GtkCaret, gtk_caret, GTK_TYPE_DRAWING_AREA)

static gboolean blink_cb(gpointer data)
{
    GtkCaret *caret = GTK_CARET(data);
    caret->blink = !caret->blink;
    gtk_widget_queue_draw(GTK_WIDGET(caret));
    return G_SOURCE_CONTINUE;
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr)
{
    GtkCaret *caret = GTK_CARET(widget);
    if (/*!caret->visible ||*/ !caret->blink)
        return FALSE;

    int width = gtk_widget_get_allocated_width(GTK_WIDGET(caret));
    int height = gtk_widget_get_allocated_height(GTK_WIDGET(caret));

    GdkRGBA bg = {0.5, 0.5, 0.5, 1.0};
    GdkWindow *gdk_wnd = gtk_widget_get_window(widget);
    if (gdk_wnd) {
        GdkPixbuf *pixbuf = gdk_pixbuf_get_from_window(gdk_wnd, 0, 0, 1, 1);
        if (pixbuf) {
            guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
            int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
            if (n_channels >= 3) {
                bg.red   = pixels[0] / 255.0;
                bg.green = pixels[1] / 255.0;
                bg.blue  = pixels[2] / 255.0;
                bg.alpha = 1.0;
            }
            g_object_unref(pixbuf);
            double lum = 0.2126 * bg.red + 0.7152 * bg.green + 0.0722 * bg.blue;
            bg = (lum < 0.5) ? (GdkRGBA){1.0, 1.0, 1.0, 1.0} : (GdkRGBA){0.0, 0.0, 0.0, 1.0};
        }
    }
    cairo_set_source_rgba(cr, bg.red, bg.green, bg.blue, 1.0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    if (gdk_wnd) {
        cairo_region_t *rg = cairo_region_create();
        GdkRectangle grc = {0, 0, 0, 0};
        cairo_region_union_rectangle(rg, &grc);
        gdk_window_input_shape_combine_region(gdk_wnd, rg, 0, 0);
        cairo_region_destroy(rg);
    }
    return FALSE;
}

static void gtk_caret_init(GtkCaret* caret)
{
    caret->parent_wgt = NULL;
    caret->blink = TRUE;
    caret->blink_timeout = g_timeout_add(DEFAULT_BLINK_INTERVAL, blink_cb, caret);
    gtk_widget_set_app_paintable(GTK_WIDGET(caret), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(caret), FALSE);
    g_signal_connect(caret, "draw", G_CALLBACK(draw_cb), NULL);
}

static void gtk_caret_dispose(GObject* object)
{
    GtkCaret *caret = GTK_CARET(object);
    if (caret->blink_timeout > 0)
        g_source_remove(caret->blink_timeout);
    G_OBJECT_CLASS(gtk_caret_parent_class)->dispose(object);
}

static void gtk_caret_class_init(GtkCaretClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = gtk_caret_dispose;
}

GtkWidget* gtk_caret_new(void)
{
    return g_object_new(GTK_TYPE_CARET, NULL);
}

GtkWidget* gtk_caret_create(GtkWidget *parent, gint width, gint height)
{
    GtkCaret *caret = (GtkCaret*)gtk_caret_new();
    if ((caret->parent_wgt = parent) != NULL)
        gtk_layout_put(GTK_LAYOUT(parent), GTK_WIDGET(caret), 0, 0);
    gtk_widget_set_size_request(GTK_WIDGET(caret), width, height);
    return GTK_WIDGET(caret);
}

void gtk_caret_destroy(GtkCaret *caret)
{
    if (caret->parent_wgt)
        gtk_container_remove(GTK_CONTAINER(caret->parent_wgt), GTK_WIDGET(caret));
    // g_object_unref(caret);
}

void gtk_caret_set_position(GtkCaret *caret, gint x, gint y)
{
    if (caret->parent_wgt) {
        gtk_layout_move(GTK_LAYOUT(caret->parent_wgt), GTK_WIDGET(caret), x, y);
        GtkWidget *toplevel = gtk_widget_get_toplevel(caret->parent_wgt);
        if (toplevel)
            gtk_widget_queue_resize(toplevel); // gtk_widget_queue_allocate(toplevel);
    }
}

void gtk_caret_show(GtkCaret *caret)
{
    gtk_widget_show(GTK_WIDGET(caret));
    if (caret->parent_wgt) {
        GtkWidget *toplevel = gtk_widget_get_toplevel(caret->parent_wgt);
        if (toplevel)
            gtk_widget_queue_resize(toplevel); // gtk_widget_queue_allocate(toplevel);
    }
}

void gtk_caret_hide(GtkCaret *caret)
{
    gtk_widget_hide(GTK_WIDGET(caret));
}
