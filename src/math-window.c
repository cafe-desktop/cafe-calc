/*
 * Copyright (C) 1987-2008 Sun Microsystems, Inc. All Rights Reserved.
 * Copyright (C) 2008-2011 Robert Ancell.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <glib/gi18n.h>
#include <ctk/ctk.h>
#include <cdk/cdkkeysyms.h>

#include "math-window.h"
#include "utility.h"

// ctk3 hack
	#ifndef CDK_KEY_F1
		#define CDK_KEY_F1 CDK_F1
	#endif

	#ifndef CDK_KEY_W
		#define CDK_KEY_W CDK_w
	#endif

	#ifndef CDK_KEY_Z
		#define CDK_KEY_Z CDK_z
	#endif

enum {
    PROP_0,
    PROP_EQUATION
};

struct MathWindowPrivate
{
    CtkWidget *menu_bar;
    MathEquation *equation;
    MathDisplay *display;
    MathButtons *buttons;
    MathPreferencesDialog *preferences_dialog;
    gboolean right_aligned;
    CtkWidget *mode_basic_menu_item;
    CtkWidget *mode_advanced_menu_item;
    CtkWidget *mode_financial_menu_item;
    CtkWidget *mode_programming_menu_item;
};

G_DEFINE_TYPE_WITH_PRIVATE (MathWindow, math_window, CTK_TYPE_WINDOW);

enum
{
    QUIT,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

MathWindow *
math_window_new(MathEquation *equation)
{
    return g_object_new(math_window_get_type(),
            "equation", equation, NULL);
}

CtkWidget *math_window_get_menu_bar(MathWindow *window)
{
    return window->priv->menu_bar;
}

MathEquation *
math_window_get_equation(MathWindow *window)
{
    g_return_val_if_fail(window != NULL, NULL);
    return window->priv->equation;
}


MathDisplay *
math_window_get_display(MathWindow *window)
{
    g_return_val_if_fail(window != NULL, NULL);
    return window->priv->display;
}


MathButtons *
math_window_get_buttons(MathWindow *window)
{
    g_return_val_if_fail(window != NULL, NULL);
    return window->priv->buttons;
}


void
math_window_critical_error(MathWindow *window, const gchar *title, const gchar *contents)
{
    CtkWidget *dialog;

    g_return_if_fail(window != NULL);
    g_return_if_fail(title != NULL);
    g_return_if_fail(contents != NULL);

    dialog = ctk_message_dialog_new(NULL, 0,
                                    CTK_MESSAGE_ERROR,
                                    CTK_BUTTONS_NONE,
                                    "%s", title);
    ctk_message_dialog_format_secondary_text(CTK_MESSAGE_DIALOG(dialog),
                                             "%s", contents);
    ctk_dialog_add_buttons(CTK_DIALOG(dialog), "ctk-quit", CTK_RESPONSE_ACCEPT, NULL);

    ctk_dialog_run(CTK_DIALOG(dialog));

    g_signal_emit(window, signals[QUIT], 0);
}

static void mode_changed_cb(CtkWidget *menu, MathWindow *window)
{
    int mode;

    if (!ctk_check_menu_item_get_active(CTK_CHECK_MENU_ITEM(menu)))
    {
        return;
    }

    mode = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menu), "calcmode"));
    math_buttons_set_mode(window->priv->buttons, mode);
}

static void show_preferences_cb(CtkMenuItem *menu, MathWindow *window)
{
    if (!window->priv->preferences_dialog)
    {
        window->priv->preferences_dialog = math_preferences_dialog_new(window->priv->equation);
        ctk_window_set_transient_for(CTK_WINDOW(window->priv->preferences_dialog), CTK_WINDOW(window));
    }

    ctk_window_present(CTK_WINDOW(window->priv->preferences_dialog));
}

static gboolean
key_press_cb(MathWindow *window, CdkEventKey *event)
{
    gboolean result;
    g_signal_emit_by_name(window->priv->display, "key-press-event", event, &result);

    if (math_buttons_get_mode (window->priv->buttons) == PROGRAMMING && (event->state & CDK_CONTROL_MASK) == CDK_CONTROL_MASK) {
        switch(event->keyval)
        {
        /* Binary */
        case CDK_KEY_b:
            math_equation_set_base (window->priv->equation, 2);
            return TRUE;
        /* Octal */
        case CDK_KEY_o:
            math_equation_set_base (window->priv->equation, 8);
            return TRUE;
        /* Decimal */
        case CDK_KEY_d:
            math_equation_set_base (window->priv->equation, 10);
            return TRUE;
        /* Hexdecimal */
        case CDK_KEY_h:
            math_equation_set_base (window->priv->equation, 16);
            return TRUE;
        }
    }

    return result;
}

static void delete_cb(MathWindow *window, CdkEvent *event)
{
    g_signal_emit(window, signals[QUIT], 0);
}

static void copy_cb(CtkWidget *widget, MathWindow *window)
{
    math_equation_copy(window->priv->equation);
}

static void paste_cb(CtkWidget *widget, MathWindow *window)
{
    math_equation_paste(window->priv->equation);
}

static void undo_cb(CtkWidget *widget, MathWindow *window)
{
    math_equation_undo(window->priv->equation);
}

static void redo_cb(CtkWidget *widget, MathWindow *window)
{
    math_equation_redo(window->priv->equation);
}

static void help_cb(CtkWidget *widget, MathWindow *window)
{
    GError *error = NULL;

    ctk_show_uri_on_window(CTK_WINDOW(window),
                           "help:cafe-calc",
                           ctk_get_current_event_time(),
                           &error);

    if (error != NULL)
    {
        CtkWidget *d;
        /* Translators: Error message displayed when unable to launch help browser */
        const char *message = _("Unable to open help file");

        d = ctk_message_dialog_new(CTK_WINDOW(window), CTK_DIALOG_MODAL | CTK_DIALOG_DESTROY_WITH_PARENT,
                CTK_MESSAGE_ERROR, CTK_BUTTONS_CLOSE, "%s", message);
        ctk_message_dialog_format_secondary_text(CTK_MESSAGE_DIALOG(d), "%s", error->message);
        g_signal_connect(d, "response", G_CALLBACK(ctk_widget_destroy), NULL);
        ctk_window_present(CTK_WINDOW (d));

        g_error_free(error);
    }
}

#define ABOUT_GROUP "About"
#define EMAILIFY(string) (g_strdelimit ((string), "%", '@'))

static void about_cb(CtkWidget* widget, MathWindow* window)
{
    const char* documenters[] = {
        N_("Sun Microsystems"),
        N_("CAFE Documentation Team"),
        NULL
    };

    /* The license this software is under (GPL2+) */
    const char* license[] = {
        N_("CAFE Calculator is free software; you can redistribute it and/or modify "
           "it under the terms of the GNU General Public License as published by "
           "the Free Software Foundation; either version 2 of the License, or "
           "(at your option) any later version."),
        N_("CAFE Calculator is distributed in the hope that it will be useful, "
           "but WITHOUT ANY WARRANTY; without even the implied warranty of "
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
           "GNU General Public License for more details."),
        N_("You should have received a copy of the GNU General Public License "
           "along with CAFE Calculator; if not, write to the Free Software Foundation, Inc., "
           "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.")
    };

    char *license_trans = g_strjoin ("\n\n", _(license[0]), _(license[1]), _(license[2]), NULL);

    GKeyFile *key_file;
    GBytes *bytes;
    const guint8 *data;
    gsize data_len;
    GError *error = NULL;
    char **authors;
    gsize n_authors = 0, i;

    bytes = g_resources_lookup_data ("/org/cafe/calculator/ui/cafe-calc.about", G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
    g_assert_no_error (error);

    data = g_bytes_get_data (bytes, &data_len);
    key_file = g_key_file_new ();
    g_key_file_load_from_data (key_file, (const char *) data, data_len, 0, &error);
    g_assert_no_error (error);

    authors = g_key_file_get_string_list (key_file, ABOUT_GROUP, "Authors", &n_authors, NULL);

    g_key_file_free (key_file);
    g_bytes_unref (bytes);

    for (i = 0; i < n_authors; ++i)
        authors[i] = EMAILIFY (authors[i]);

    const char **p;

    for (p = documenters; *p; ++p)
        *p = _(*p);

    ctk_show_about_dialog(CTK_WINDOW(window),
        "program-name", _("CAFE Calculator"),
        "version", VERSION,
        "title", _("About CAFE Calculator"),
        "copyright", _("Copyright \xc2\xa9 1986–2010 The GCalctool authors\n"
                       "Copyright \xc2\xa9 2011-2020 CAFE developers"),
        "license", license_trans,
        "comments", _("Calculator with financial and scientific modes."),
        "authors", authors,
        "documenters", documenters,
        "translator_credits", _("translator-credits"),
        "wrap-license", TRUE,
        "website", "http://cafe-desktop.org",
        "icon-name", "accessories-calculator",
        "logo-icon-name", "accessories-calculator",
        NULL);

    g_strfreev (authors);
    g_free (license_trans);
}

static void
scroll_changed_cb(CtkAdjustment *adjustment, MathWindow *window)
{
    if (window->priv->right_aligned)
        ctk_adjustment_set_value(adjustment, ctk_adjustment_get_upper(adjustment) - ctk_adjustment_get_page_size(adjustment));
}


static void
scroll_value_changed_cb(CtkAdjustment *adjustment, MathWindow *window)
{
    if (ctk_adjustment_get_value(adjustment) == ctk_adjustment_get_upper(adjustment) - ctk_adjustment_get_page_size(adjustment))
        window->priv->right_aligned = TRUE;
    else
        window->priv->right_aligned = FALSE;
}

static void button_mode_changed_cb(MathButtons *buttons, GParamSpec *spec, MathWindow *window)
{
    CtkWidget *menu;

    switch (math_buttons_get_mode(buttons))
    {
        default:
        case BASIC:
            menu = window->priv->mode_basic_menu_item;
            break;
        case ADVANCED:
            menu = window->priv->mode_advanced_menu_item;
            break;
        case FINANCIAL:
            menu = window->priv->mode_financial_menu_item;
            break;
        case PROGRAMMING:
            menu = window->priv->mode_programming_menu_item;
            break;
    }

    ctk_check_menu_item_set_active(CTK_CHECK_MENU_ITEM(menu), TRUE);
    g_settings_set_enum(g_settings_var, "button-mode", math_buttons_get_mode(buttons));
}

static CtkWidget *add_menu(CtkWidget *menu_bar, const gchar *name)
{
    CtkWidget *menu_item;
    CtkWidget *menu;

    menu_item = ctk_menu_item_new_with_mnemonic(name);
    ctk_menu_shell_append(CTK_MENU_SHELL(menu_bar), menu_item);
    ctk_widget_show(menu_item);
    menu = ctk_menu_new();
    ctk_menu_set_reserve_toggle_size(CTK_MENU(menu),FALSE);
    ctk_menu_item_set_submenu(CTK_MENU_ITEM(menu_item), menu);

    return menu;
}

static void quit_cb(CtkWidget* widget, MathWindow* window)
{
    g_signal_emit(window, signals[QUIT], 0);
}

static CtkWidget *add_menu_item(CtkWidget *menu, CtkWidget *menu_item, GCallback callback, gpointer callback_data)
{
    ctk_menu_shell_append(CTK_MENU_SHELL(menu), menu_item);
    ctk_widget_show(menu_item);

    if (callback)
    {
        g_signal_connect(G_OBJECT(menu_item), "activate", callback, callback_data);
    }

    return menu_item;
}

static CtkWidget *radio_menu_item_new(GSList **group, const gchar *name)
{
    CtkWidget *menu_item = ctk_radio_menu_item_new_with_mnemonic(*group, name);

    *group = ctk_radio_menu_item_get_group(CTK_RADIO_MENU_ITEM(menu_item));

    return menu_item;
}

static CtkWidget *ctk_image_menu_item_new_from_icon (const gchar   *icon_name,
                                                     const gchar   *label_name,
                                                     CtkAccelGroup *accel_group)
{
    gchar *concat = g_strconcat (label_name, "     ", NULL);
    CtkWidget *box = ctk_box_new (CTK_ORIENTATION_HORIZONTAL, 6);
    CtkWidget *icon = ctk_image_new_from_icon_name (icon_name, CTK_ICON_SIZE_MENU);
    CtkWidget *label = ctk_accel_label_new (concat);
    CtkWidget *menu_item = ctk_menu_item_new ();

    g_free (concat);

    ctk_container_add (CTK_CONTAINER (box), icon);

    ctk_label_set_use_underline (CTK_LABEL (label), TRUE);
    ctk_label_set_xalign (CTK_LABEL (label), 0.0);

    ctk_accel_label_set_accel_widget (CTK_ACCEL_LABEL (label), menu_item);

    ctk_box_pack_end (CTK_BOX (box), label, TRUE, TRUE, 0);

    ctk_container_add (CTK_CONTAINER (menu_item), box);

    ctk_widget_show_all (menu_item);

    return menu_item;
}

static void create_menu(MathWindow* window)
{
    CtkAccelGroup* accel_group;
    CtkWidget* menu;
    CtkWidget* menu_item;
    GSList* group = NULL;

    accel_group = ctk_accel_group_new();
    ctk_window_add_accel_group(CTK_WINDOW(window), accel_group);

    /* Calculator menu */
    #define CALCULATOR_MENU_LABEL _("_Calculator")
    /* Mode menu */
    #define MODE_MENU_LABEL _("_Mode")
    /* Help menu label */
    #define HELP_MENU_LABEL _("_Help")
    /* Basic menu label */
    #define MODE_BASIC_LABEL _("_Basic")
    /* Advanced menu label */
    #define MODE_ADVANCED_LABEL _("_Advanced")
    /* Financial menu label */
    #define MODE_FINANCIAL_LABEL _("_Financial")
    /* Programming menu label */
    #define MODE_PROGRAMMING_LABEL _("_Programming")
    /* Help>Contents menu label */
    #define HELP_CONTENTS_LABEL _("_Contents")

    menu = add_menu(window->priv->menu_bar, CALCULATOR_MENU_LABEL);
    menu_item = add_menu_item(menu, ctk_image_menu_item_new_from_icon("edit-copy",_("_Copy"), accel_group), G_CALLBACK(copy_cb), window);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_C, CDK_CONTROL_MASK, CTK_ACCEL_VISIBLE);
    menu_item = add_menu_item(menu, ctk_image_menu_item_new_from_icon("edit-paste",_("_Paste"), accel_group), G_CALLBACK(paste_cb), window);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_V, CDK_CONTROL_MASK, CTK_ACCEL_VISIBLE);
    menu_item = add_menu_item(menu, ctk_image_menu_item_new_from_icon("edit-undo",_("_Undo"), accel_group), G_CALLBACK(undo_cb), window);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_Z, CDK_CONTROL_MASK, CTK_ACCEL_VISIBLE);
    menu_item = add_menu_item(menu, ctk_image_menu_item_new_from_icon("edit-redo",_("_Redo"), accel_group), G_CALLBACK(redo_cb), window);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_Z, CDK_CONTROL_MASK | CDK_SHIFT_MASK, CTK_ACCEL_VISIBLE);
    add_menu_item(menu, ctk_separator_menu_item_new(), NULL, NULL);
    add_menu_item(menu, ctk_image_menu_item_new_from_icon("preferences-desktop",_("_Preferences"), accel_group), G_CALLBACK(show_preferences_cb), window);
    add_menu_item(menu, ctk_separator_menu_item_new(), NULL, NULL);
    menu_item = add_menu_item(menu, ctk_image_menu_item_new_from_icon("application-exit",_("_Quit"), accel_group), G_CALLBACK(quit_cb), window);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_Q, CDK_CONTROL_MASK, CTK_ACCEL_VISIBLE);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_W, CDK_CONTROL_MASK, CTK_ACCEL_VISIBLE);

    menu = add_menu(window->priv->menu_bar, MODE_MENU_LABEL);
    window->priv->mode_basic_menu_item = add_menu_item(menu, radio_menu_item_new(&group, MODE_BASIC_LABEL), G_CALLBACK(mode_changed_cb), window);
    g_object_set_data(G_OBJECT(window->priv->mode_basic_menu_item), "calcmode", GINT_TO_POINTER(BASIC));
    window->priv->mode_advanced_menu_item = add_menu_item(menu, radio_menu_item_new(&group, MODE_ADVANCED_LABEL), G_CALLBACK(mode_changed_cb), window);
    g_object_set_data(G_OBJECT(window->priv->mode_advanced_menu_item), "calcmode", GINT_TO_POINTER(ADVANCED));
    window->priv->mode_financial_menu_item = add_menu_item(menu, radio_menu_item_new(&group, MODE_FINANCIAL_LABEL), G_CALLBACK(mode_changed_cb), window);
    g_object_set_data(G_OBJECT(window->priv->mode_financial_menu_item), "calcmode", GINT_TO_POINTER(FINANCIAL));
    window->priv->mode_programming_menu_item = add_menu_item(menu, radio_menu_item_new(&group, MODE_PROGRAMMING_LABEL), G_CALLBACK(mode_changed_cb), window);
    g_object_set_data(G_OBJECT(window->priv->mode_programming_menu_item), "calcmode", GINT_TO_POINTER(PROGRAMMING));

    menu = add_menu(window->priv->menu_bar, HELP_MENU_LABEL);
    menu_item = add_menu_item(menu, ctk_image_menu_item_new_from_icon("help-browser", HELP_CONTENTS_LABEL, accel_group), G_CALLBACK(help_cb), window);
    ctk_widget_add_accelerator(menu_item, "activate", accel_group, CDK_KEY_F1, 0, CTK_ACCEL_VISIBLE);
    add_menu_item(menu, ctk_image_menu_item_new_from_icon("help-about",_("_About"), accel_group), G_CALLBACK(about_cb), window);
}

static void
create_gui(MathWindow *window)
{
    CtkWidget *main_vbox, *vbox;
    CtkWidget *scrolled_window;

    main_vbox = ctk_box_new(CTK_ORIENTATION_VERTICAL, 0);
    ctk_container_add(CTK_CONTAINER(window), main_vbox);
    ctk_widget_show(main_vbox);

    window->priv->menu_bar = ctk_menu_bar_new();
    ctk_box_pack_start(CTK_BOX(main_vbox), window->priv->menu_bar, TRUE, TRUE, 0);
    ctk_widget_show(window->priv->menu_bar);

    create_menu(window);

    vbox = ctk_box_new(CTK_ORIENTATION_VERTICAL, 6);
    ctk_container_set_border_width(CTK_CONTAINER(vbox), 6);
    ctk_box_pack_start(CTK_BOX(main_vbox), vbox, TRUE, TRUE, 0);
    ctk_widget_show(vbox);

    scrolled_window = ctk_scrolled_window_new(NULL, NULL);
    ctk_scrolled_window_set_policy(CTK_SCROLLED_WINDOW(scrolled_window), CTK_POLICY_AUTOMATIC, CTK_POLICY_NEVER);
    ctk_scrolled_window_set_shadow_type(CTK_SCROLLED_WINDOW(scrolled_window), CTK_SHADOW_IN);
    ctk_box_pack_start(CTK_BOX(vbox), CTK_WIDGET(scrolled_window), TRUE, TRUE, 0);
    g_signal_connect(ctk_scrolled_window_get_hadjustment(CTK_SCROLLED_WINDOW(scrolled_window)), "changed", G_CALLBACK(scroll_changed_cb), window);
    g_signal_connect(ctk_scrolled_window_get_hadjustment(CTK_SCROLLED_WINDOW(scrolled_window)), "value-changed", G_CALLBACK(scroll_value_changed_cb), window);
    window->priv->right_aligned = TRUE;
    ctk_widget_show(scrolled_window);

    window->priv->display = math_display_new_with_equation(window->priv->equation);
    ctk_container_add(CTK_CONTAINER(scrolled_window), CTK_WIDGET(window->priv->display));
    ctk_widget_show(CTK_WIDGET(window->priv->display));

    window->priv->buttons = math_buttons_new(window->priv->equation);
    g_signal_connect(window->priv->buttons, "notify::mode", G_CALLBACK(button_mode_changed_cb), window);
    button_mode_changed_cb(window->priv->buttons, NULL, window);
    ctk_box_pack_start(CTK_BOX(vbox), CTK_WIDGET(window->priv->buttons), TRUE, TRUE, 0);
    ctk_widget_show(CTK_WIDGET(window->priv->buttons));
}


static void
math_window_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    MathWindow *self;

    self = MATH_WINDOW(object);

    switch (prop_id) {
    case PROP_EQUATION:
        self->priv->equation = g_value_get_object(value);
        create_gui(self);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


static void
math_window_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    MathWindow *self;

    self = MATH_WINDOW(object);

    switch (prop_id) {
    case PROP_EQUATION:
        g_value_set_object(value, self->priv->equation);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


static void
math_window_class_init(MathWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = math_window_get_property;
    object_class->set_property = math_window_set_property;

    g_object_class_install_property(object_class,
                                    PROP_EQUATION,
                                    g_param_spec_object("equation",
                                                        "equation",
                                                        "Equation being calculated",
                                                        math_equation_get_type(),
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    signals[QUIT] = g_signal_new("quit",
                                 G_TYPE_FROM_CLASS (klass),
                                 G_SIGNAL_RUN_LAST,
                                 G_STRUCT_OFFSET (MathWindowClass, quit),
                                 NULL, NULL,
                                 g_cclosure_marshal_VOID__VOID,
                                 G_TYPE_NONE, 0);
}


static void
math_window_init(MathWindow *window)
{
    window->priv = math_window_get_instance_private (window);
    ctk_window_set_title(CTK_WINDOW(window),
                         /* Title of main window */
                         _("Calculator"));
    ctk_window_set_icon_name(CTK_WINDOW(window), "accessories-calculator");
    ctk_window_set_role(CTK_WINDOW(window), "cafe-calc");
    ctk_window_set_resizable(CTK_WINDOW(window), FALSE);
    g_signal_connect_after(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_cb), NULL);
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_cb), NULL);
}
