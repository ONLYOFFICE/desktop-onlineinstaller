/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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
