/*
 * Copyright (C) 2008-2011 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <glib/gi18n.h>
#include <cdk/cdkkeysyms.h>

#include "math-variable-popup.h"

enum {
    PROP_0,
    PROP_EQUATION
};

struct MathVariablePopupPrivate
{
    MathEquation *equation;

    CtkWidget *vbox;
    CtkWidget *variable_name_entry;
    CtkWidget *add_variable_button;
};

G_DEFINE_TYPE_WITH_PRIVATE (MathVariablePopup, math_variable_popup, CTK_TYPE_WINDOW);

MathVariablePopup *
math_variable_popup_new(MathEquation *equation)
{
    return g_object_new(math_variable_popup_get_type(), "equation", equation, NULL);
}


static void
variable_focus_out_event_cb(CtkWidget *widget, CdkEventFocus *event, MathVariablePopup *popup)
{
    ctk_widget_destroy(widget);
}


static void
insert_variable_cb(CtkWidget *widget, MathVariablePopup *popup)
{
    const gchar *name;

    name = g_object_get_data(G_OBJECT(widget), "variable_name");
    math_equation_insert(popup->priv->equation, name);

    ctk_widget_destroy(ctk_widget_get_toplevel(widget));
}


static gboolean
variable_name_key_press_cb(CtkWidget *widget, CdkEventKey *event, MathVariablePopup *popup)
{
    /* Can't have whitespace in names, so replace with underscores */
    if (event->keyval == GDK_KEY_space || event->keyval == GDK_KEY_KP_Space)
        event->keyval = GDK_KEY_underscore;

    return FALSE;
}


static void
variable_name_changed_cb(CtkWidget *widget, MathVariablePopup *popup)
{
    const gchar *text = ctk_entry_get_text(CTK_ENTRY(popup->priv->variable_name_entry));
    ctk_widget_set_sensitive(popup->priv->add_variable_button, text[0] != '\0');
}


static void
add_variable_cb(CtkWidget *widget, MathVariablePopup *popup)
{
    const gchar *name;
    MPNumber z;

    name = ctk_entry_get_text(CTK_ENTRY(popup->priv->variable_name_entry));
    if (name[0] == '\0')
        return;

    if (math_equation_get_number(popup->priv->equation, &z))
        math_variables_set(math_equation_get_variables(popup->priv->equation), name, &z);
    else if (math_equation_is_result(popup->priv->equation))
        math_variables_set(math_equation_get_variables(popup->priv->equation), name, math_equation_get_answer(popup->priv->equation));
    else
        g_warning("Can't add variable %s, the display is not a number", name);

    ctk_widget_destroy(ctk_widget_get_toplevel(widget));
}


static void
save_variable_cb(CtkWidget *widget, MathVariablePopup *popup)
{
    const gchar *name;
    MPNumber z;

    name = g_object_get_data(G_OBJECT(widget), "variable_name");
    if (math_equation_get_number(popup->priv->equation, &z))
        math_variables_set(math_equation_get_variables(popup->priv->equation), name, &z);
    else if (math_equation_is_result(popup->priv->equation))
        math_variables_set(math_equation_get_variables(popup->priv->equation), name, math_equation_get_answer(popup->priv->equation));
    else
        g_warning("Can't save variable %s, the display is not a number", name);

    ctk_widget_destroy(ctk_widget_get_toplevel(widget));
}


static void
delete_variable_cb(CtkWidget *widget, MathVariablePopup *popup)
{
    const gchar *name;

    name = g_object_get_data(G_OBJECT(widget), "variable_name");
    math_variables_delete(math_equation_get_variables(popup->priv->equation), name);

    ctk_widget_destroy(ctk_widget_get_toplevel(widget));
}


static CtkWidget *
make_variable_entry(MathVariablePopup *popup, const gchar *name, const MPNumber *value, gboolean writable)
{
    CtkWidget *hbox, *button, *label;
    gchar *text;

    hbox = ctk_box_new(CTK_ORIENTATION_HORIZONTAL, 6);

    if (value)
    {
        gchar *value_text = mp_serializer_to_string(math_equation_get_serializer(popup->priv->equation), value);
        text = g_strdup_printf("<b>%s</b> = %s", name, value_text);
        g_free (value_text);
    }
    else
        text = g_strdup_printf("<b>%s</b>", name);

    button = ctk_button_new();
    g_object_set_data(G_OBJECT(button), "variable_name", g_strdup(name)); // FIXME: These will all leak memory
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(insert_variable_cb), popup);
    ctk_button_set_relief(CTK_BUTTON(button), CTK_RELIEF_NONE);
    ctk_box_pack_start(CTK_BOX(hbox), button, TRUE, TRUE, 0);
    ctk_widget_show(button);

    label = ctk_label_new(text);
    g_free(text);
    ctk_label_set_use_markup(CTK_LABEL(label), TRUE);
    ctk_widget_set_halign (label, CTK_ALIGN_START);
    ctk_container_add(CTK_CONTAINER(button), label);
    ctk_widget_show(label);

    if (writable)
    {
        CtkWidget *image;

        button = ctk_button_new();
        g_object_set_data(G_OBJECT(button), "variable_name", g_strdup(name));
        image = ctk_image_new_from_icon_name("document-save", CTK_ICON_SIZE_BUTTON);
        ctk_container_add(CTK_CONTAINER(button), image);
        ctk_box_pack_start(CTK_BOX(hbox), button, FALSE, TRUE, 0);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(save_variable_cb), popup);
        ctk_widget_show(image);
        ctk_widget_show(button);

        button = ctk_button_new();
        g_object_set_data(G_OBJECT(button), "variable_name", g_strdup(name));
        image = ctk_image_new_from_icon_name("edit-delete", CTK_ICON_SIZE_BUTTON);
        ctk_container_add(CTK_CONTAINER(button), image);
        ctk_box_pack_start(CTK_BOX(hbox), button, FALSE, TRUE, 0);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(delete_variable_cb), popup);
        ctk_widget_show(image);
        ctk_widget_show(button);
    }

    return hbox;
}


static void
math_variable_popup_set_property(GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    MathVariablePopup *self;
    gchar **names;
    int i;
    CtkWidget *entry, *image;

    self = MATH_VARIABLE_POPUP(object);

    switch (prop_id) {
    case PROP_EQUATION:
        self->priv->equation = g_value_get_object(value);

        names = math_variables_get_names(math_equation_get_variables(self->priv->equation));
        for (i = 0; names[i]; i++) {
            MPNumber *value;

            value = math_variables_get(math_equation_get_variables(self->priv->equation), names[i]);
            entry = make_variable_entry(self, names[i], value, TRUE);
            ctk_widget_show(entry);
            ctk_box_pack_start(CTK_BOX(self->priv->vbox), entry, FALSE, TRUE, 0);
        }
        g_strfreev(names);

        entry = ctk_box_new(CTK_ORIENTATION_HORIZONTAL, 6);
        ctk_widget_show(entry);

        // TODO: Show greyed "variable name" text to give user a hint how to use
        self->priv->variable_name_entry = ctk_entry_new();
        g_signal_connect(G_OBJECT(self->priv->variable_name_entry), "key-press-event", G_CALLBACK(variable_name_key_press_cb), self);
        g_signal_connect(G_OBJECT(self->priv->variable_name_entry), "changed", G_CALLBACK(variable_name_changed_cb), self);
        g_signal_connect(G_OBJECT(self->priv->variable_name_entry), "activate", G_CALLBACK(add_variable_cb), self);
        ctk_box_pack_start(CTK_BOX(entry), self->priv->variable_name_entry, TRUE, TRUE, 0);
        ctk_widget_show(self->priv->variable_name_entry);

        self->priv->add_variable_button = ctk_button_new();
        ctk_widget_set_sensitive(self->priv->add_variable_button, FALSE);
        g_signal_connect(G_OBJECT(self->priv->add_variable_button), "clicked", G_CALLBACK(add_variable_cb), self);
        image = ctk_image_new_from_icon_name("list-add", CTK_ICON_SIZE_BUTTON);
        ctk_container_add(CTK_CONTAINER(self->priv->add_variable_button), image);
        ctk_box_pack_start(CTK_BOX(entry), self->priv->add_variable_button, FALSE, TRUE, 0);
        ctk_widget_show(image);
        ctk_widget_show(self->priv->add_variable_button);
        ctk_box_pack_end(CTK_BOX(self->priv->vbox), entry, FALSE, TRUE, 0);

        entry = make_variable_entry(self, "rand", NULL, FALSE);
        ctk_widget_show(entry);
        ctk_box_pack_end(CTK_BOX(self->priv->vbox), entry, FALSE, TRUE, 0);

        entry = make_variable_entry(self, "ans", math_equation_get_answer(self->priv->equation), FALSE);
        ctk_widget_show(entry);
        ctk_box_pack_end(CTK_BOX(self->priv->vbox), entry, FALSE, TRUE, 0);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


static void
math_variable_popup_get_property(GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    MathVariablePopup *self;

    self = MATH_VARIABLE_POPUP(object);

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
math_variable_popup_class_init(MathVariablePopupClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = math_variable_popup_get_property;
    object_class->set_property = math_variable_popup_set_property;

    g_object_class_install_property(object_class,
                                    PROP_EQUATION,
                                    g_param_spec_object("equation",
                                                        "equation",
                                                        "Equation being controlled",
                                                        math_equation_get_type(),
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
math_variable_popup_init(MathVariablePopup *popup)
{
    popup->priv = math_variable_popup_get_instance_private (popup);

    ctk_window_set_decorated(CTK_WINDOW(popup), FALSE);
    ctk_window_set_skip_taskbar_hint(CTK_WINDOW(popup), TRUE);

    ctk_container_set_border_width(CTK_CONTAINER(popup), 6);

    /* Destroy this window when it loses focus */
    g_signal_connect(G_OBJECT(popup), "focus-out-event", G_CALLBACK(variable_focus_out_event_cb), popup);

    popup->priv->vbox = ctk_box_new(CTK_ORIENTATION_VERTICAL, 6);
    ctk_box_set_homogeneous(CTK_BOX(popup->priv->vbox), TRUE);
    ctk_container_add(CTK_CONTAINER(popup), popup->priv->vbox);
    ctk_widget_show(popup->priv->vbox);
}
