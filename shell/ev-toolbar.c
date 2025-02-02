/* ev-toolbar.h
 *  this file is part of evince, a gnome document viewer
 *
 * Copyright (C) 2012-2014 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2014-2018 Germán Poo-Caamaño <gpoo@gnome.org>
 *
 * Evince is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Evince is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>

#include "ev-toolbar.h"

#include "ev-stock-icons.h"
#include "ev-zoom-action.h"
#include "ev-application.h"
#include "ev-page-action-widget.h"
#include <math.h>

enum
{
        PROP_0,
        PROP_WINDOW
};

typedef struct {
	EvWindow  *window;

	HdyHeaderBar *header_bar;

	GtkWidget *open_button;
	GtkWidget *sidebar_button;
	GtkWidget *page_selector;
	GtkWidget *annots_button;
	GtkWidget *zoom_action;
	GtkWidget *find_button;
	GtkWidget *action_menu_button;
        GtkWidget *zoom_revealer;
        GtkWidget *page_annots_revealer;

	EvToolbarMode toolbar_mode;
} EvToolbarPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (EvToolbar, ev_toolbar, GTK_TYPE_BIN)

#define GET_PRIVATE(o) ev_toolbar_get_instance_private (o)

static void
ev_toolbar_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
        EvToolbar *ev_toolbar = EV_TOOLBAR (object);
	EvToolbarPrivate *priv = GET_PRIVATE (ev_toolbar);

        switch (prop_id) {
        case PROP_WINDOW:
                priv->window = g_value_get_object (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
ev_toolbar_zoom_selector_activated (GtkWidget *zoom_action,
				    EvToolbar *ev_toolbar)
{
	EvToolbarPrivate *priv = GET_PRIVATE (ev_toolbar);

        ev_window_focus_view (priv->window);
}

static void
ev_toolbar_find_button_sensitive_changed (GtkWidget  *find_button,
					  GParamSpec *pspec,
					  EvToolbar *ev_toolbar)
{
        GtkWidget *image;

        if (gtk_widget_is_sensitive (find_button)) {
                gtk_widget_set_tooltip_text (find_button,
                                             _("Find a word or phrase in the document"));
		image = gtk_image_new_from_icon_name ("edit-find-symbolic",
						      GTK_ICON_SIZE_MENU);
		gtk_button_set_image (GTK_BUTTON (find_button), image);
	} else {
                gtk_widget_set_tooltip_text (find_button,
                                             _("Search not available for this document"));
		image = gtk_image_new_from_icon_name (EV_STOCK_FIND_UNSUPPORTED,
						      GTK_ICON_SIZE_MENU);
		gtk_button_set_image (GTK_BUTTON (find_button), image);
	}
}

static void
ev_toolbar_constructed (GObject *object)
{
        EvToolbar      *ev_toolbar = EV_TOOLBAR (object);
	EvToolbarPrivate *priv = GET_PRIVATE (ev_toolbar);

        G_OBJECT_CLASS (ev_toolbar_parent_class)->constructed (object);

        ev_page_action_widget_set_model (EV_PAGE_ACTION_WIDGET (priv->page_selector),
                                         ev_window_get_document_model (priv->window));

	ev_zoom_action_set_model (EV_ZOOM_ACTION (priv->zoom_action),
				  ev_window_get_document_model (priv->window));
}

static void
ev_toolbar_class_init (EvToolbarClass *klass)
{
        GObjectClass *g_object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        g_object_class->set_property = ev_toolbar_set_property;
        g_object_class->constructed = ev_toolbar_constructed;

	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/evince/ui/ev-toolbar.ui");
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, header_bar);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, open_button);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, sidebar_button);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, page_selector);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, annots_button);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, action_menu_button);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, find_button);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, zoom_action);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, page_annots_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, EvToolbar, zoom_revealer);
	gtk_widget_class_bind_template_callback (widget_class, ev_toolbar_find_button_sensitive_changed);
	gtk_widget_class_bind_template_callback (widget_class, ev_toolbar_zoom_selector_activated);

        g_object_class_install_property (g_object_class,
                                         PROP_WINDOW,
                                         g_param_spec_object ("window",
                                                              "Window",
                                                              "The evince window",
                                                              EV_TYPE_WINDOW,
                                                              G_PARAM_WRITABLE |
                                                              G_PARAM_CONSTRUCT_ONLY |
                                                              G_PARAM_STATIC_STRINGS));
}

static void
ev_toolbar_init (EvToolbar *ev_toolbar)
{
	EvToolbarPrivate *priv = GET_PRIVATE (ev_toolbar);

	priv->toolbar_mode = EV_TOOLBAR_MODE_NORMAL;

        /* Ensure GTK+ private types used by the template
         * definition before calling gtk_widget_init_template() */
        g_type_ensure (EV_TYPE_PAGE_ACTION_WIDGET);
        g_type_ensure (EV_TYPE_ZOOM_ACTION);

	gtk_widget_init_template (GTK_WIDGET (ev_toolbar));
}

GtkWidget *
ev_toolbar_new (EvWindow *window)
{
        g_return_val_if_fail (EV_IS_WINDOW (window), NULL);

        return GTK_WIDGET (g_object_new (EV_TYPE_TOOLBAR,
                                         "window", window,
                                         NULL));
}

void
ev_toolbar_action_menu_toggle (EvToolbar *ev_toolbar)
{
	EvToolbarPrivate *priv;
	gboolean is_active;

        g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

        priv = GET_PRIVATE (ev_toolbar);
        is_active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->action_menu_button));

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->action_menu_button),
                                      !is_active);
}

GtkWidget *
ev_toolbar_get_page_selector (EvToolbar *ev_toolbar)
{
	EvToolbarPrivate *priv;

        g_return_val_if_fail (EV_IS_TOOLBAR (ev_toolbar), NULL);

        priv = GET_PRIVATE (ev_toolbar);

        return priv->page_selector;
}

HdyHeaderBar *
ev_toolbar_get_header_bar (EvToolbar *ev_toolbar)
{
	EvToolbarPrivate *priv;

        g_return_val_if_fail (EV_IS_TOOLBAR (ev_toolbar), NULL);

        priv = GET_PRIVATE (ev_toolbar);

        return priv->header_bar;
}

void
ev_toolbar_set_mode (EvToolbar     *ev_toolbar,
                     EvToolbarMode  mode)
{
        EvToolbarPrivate *priv;

        g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

        priv = GET_PRIVATE (ev_toolbar);
        priv->toolbar_mode = mode;

        switch (mode) {
        case EV_TOOLBAR_MODE_NORMAL:
        case EV_TOOLBAR_MODE_FULLSCREEN:
                gtk_widget_show (priv->sidebar_button);
                gtk_widget_show (priv->action_menu_button);
                gtk_widget_show (priv->zoom_revealer);
                gtk_widget_show (priv->page_annots_revealer);
                gtk_widget_show (priv->find_button);
                gtk_widget_hide (priv->open_button);
                break;
	case EV_TOOLBAR_MODE_RECENT_VIEW:
                gtk_widget_hide (priv->sidebar_button);
                gtk_widget_hide (priv->action_menu_button);
                gtk_widget_hide (priv->zoom_revealer);
                gtk_widget_hide (priv->page_annots_revealer);
                gtk_widget_hide (priv->find_button);
                gtk_widget_show (priv->open_button);
                break;
        }
}

EvToolbarMode
ev_toolbar_get_mode (EvToolbar *ev_toolbar)
{
        EvToolbarPrivate *priv;

        g_return_val_if_fail (EV_IS_TOOLBAR (ev_toolbar), EV_TOOLBAR_MODE_NORMAL);

        priv = GET_PRIVATE (ev_toolbar);

        return priv->toolbar_mode;
}

void
ev_toolbar_set_sidebar_visible (EvToolbar *ev_toolbar,
                                gboolean   visible)
{
	EvToolbarPrivate *priv;

	g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

	priv = GET_PRIVATE (ev_toolbar);
	visible = !!visible;

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->sidebar_button), visible);
}

void
ev_toolbar_set_mobile (EvToolbar *ev_toolbar,
                       gboolean   mobile)
{
	EvToolbarPrivate *priv;

	g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

	priv = GET_PRIVATE (ev_toolbar);
	mobile = !!mobile;

	gtk_revealer_set_reveal_child (GTK_REVEALER (priv->zoom_revealer), !mobile);
	gtk_revealer_set_reveal_child (GTK_REVEALER (priv->page_annots_revealer), !mobile);

	if (mobile)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->annots_button), FALSE);
}
