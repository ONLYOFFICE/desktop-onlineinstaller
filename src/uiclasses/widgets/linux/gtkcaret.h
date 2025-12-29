#ifndef GTKCARET_H
#define GTKCARET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_CARET (gtk_caret_get_type())
#define GTK_CARET(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_CARET, GtkCaret))
// #define GTK_CARET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_CARET, GtkCaretClass))
// #define IS_GTK_CARET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_CARET))
// #define IS_GTK_CARET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_CARET))

typedef struct _GtkCaret      GtkCaret;
typedef struct _GtkCaretClass GtkCaretClass;

GType gtk_caret_get_type(void);
// GtkWidget* gtk_caret_new(void);
GtkWidget* gtk_caret_create(GtkWidget *parent, gint width, gint height);
void gtk_caret_destroy(GtkCaret *caret);
void gtk_caret_set_position(GtkCaret *caret, gint x, gint y);
void gtk_caret_show(GtkCaret *caret);
void gtk_caret_hide(GtkCaret *caret);

G_END_DECLS

#endif // GTKCARET_H
