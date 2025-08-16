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
#include <ctk/ctk.h>

#include "math-preferences.h"
#include "utility.h"


enum {
    PROP_0,
    PROP_EQUATION
};

struct MathPreferencesDialogPrivate
{
    MathEquation *equation;
    CtkBuilder *ui;
};

G_DEFINE_TYPE_WITH_PRIVATE (MathPreferencesDialog, math_preferences, CTK_TYPE_DIALOG);

#define UI_DIALOGS_RESOURCE_PATH "/org/cafe/calculator/ui/preferences.ui"
#define GET_WIDGET(ui, name) \
          CTK_WIDGET(ctk_builder_get_object(ui, name))


MathPreferencesDialog *
math_preferences_dialog_new(MathEquation *equation)
{
    return g_object_new(math_preferences_get_type(), "equation", equation, NULL);
}


static void
preferences_response_cb (CtkWidget *widget,
			 gint       id G_GNUC_UNUSED)
{
    ctk_widget_hide(widget);
}


static gboolean
preferences_dialog_delete_cb (CtkWidget *widget,
			      CdkEvent  *event G_GNUC_UNUSED)
{
    preferences_response_cb(widget, 0);
    return TRUE;
}


void number_format_combobox_changed_cb (CtkWidget             *combo,
					MathPreferencesDialog *dialog)
{
    MpDisplayFormat value;
    CtkTreeModel *model;
    CtkTreeIter iter;

    model = ctk_combo_box_get_model(CTK_COMBO_BOX(combo));
    ctk_combo_box_get_active_iter(CTK_COMBO_BOX(combo), &iter);
    ctk_tree_model_get(model, &iter, 1, &value, -1);
    math_equation_set_number_format(dialog->priv->equation, value);
}


void angle_unit_combobox_changed_cb (CtkWidget             *combo,
				     MathPreferencesDialog *dialog)
{
    MPAngleUnit value;
    CtkTreeModel *model;
    CtkTreeIter iter;

    model = ctk_combo_box_get_model(CTK_COMBO_BOX(combo));
    ctk_combo_box_get_active_iter(CTK_COMBO_BOX(combo), &iter);
    ctk_tree_model_get(model, &iter, 1, &value, -1);
    math_equation_set_angle_units(dialog->priv->equation, value);
}


void word_size_combobox_changed_cb (CtkWidget             *combo,
				    MathPreferencesDialog *dialog)
{
    gint value;
    CtkTreeModel *model;
    CtkTreeIter iter;

    model = ctk_combo_box_get_model(CTK_COMBO_BOX(combo));
    ctk_combo_box_get_active_iter(CTK_COMBO_BOX(combo), &iter);
    ctk_tree_model_get(model, &iter, 1, &value, -1);
    math_equation_set_word_size(dialog->priv->equation, value);
}


void decimal_places_spin_change_value_cb (CtkWidget             *spin,
					  MathPreferencesDialog *dialog)
{
    gint value = 0;

    value = ctk_spin_button_get_value_as_int(CTK_SPIN_BUTTON(spin));
    math_equation_set_accuracy(dialog->priv->equation, value);
}


void thousands_separator_check_toggled_cb (CtkWidget             *check,
					   MathPreferencesDialog *dialog)
{
    gboolean value;

    value = ctk_toggle_button_get_active(CTK_TOGGLE_BUTTON(check));
    math_equation_set_show_thousands_separators(dialog->priv->equation, value);
}


void trailing_zeroes_check_toggled_cb (CtkWidget             *check,
				       MathPreferencesDialog *dialog)
{
    gboolean value;

    value = ctk_toggle_button_get_active(CTK_TOGGLE_BUTTON(check));
    math_equation_set_show_trailing_zeroes(dialog->priv->equation, value);
}


static void
set_combo_box_from_int(CtkWidget *combo, int value)
{
    CtkTreeModel *model;
    CtkTreeIter iter;
    gboolean valid;

    model = ctk_combo_box_get_model(CTK_COMBO_BOX(combo));
    valid = ctk_tree_model_get_iter_first(model, &iter);

    while (valid) {
        gint v;

        ctk_tree_model_get(model, &iter, 1, &v, -1);
        if (v == value)
            break;
        valid = ctk_tree_model_iter_next(model, &iter);
    }
    if (!valid)
        valid = ctk_tree_model_get_iter_first(model, &iter);

    ctk_combo_box_set_active_iter(CTK_COMBO_BOX(combo), &iter);
}


static void
accuracy_cb (MathEquation          *equation,
	     GParamSpec            *spec G_GNUC_UNUSED,
	     MathPreferencesDialog *dialog)
{
    ctk_spin_button_set_value(CTK_SPIN_BUTTON(ctk_builder_get_object(dialog->priv->ui, "decimal_places_spin")),
                              math_equation_get_accuracy(equation));
    g_settings_set_int(g_settings_var, "accuracy", math_equation_get_accuracy(equation));
}


static void
show_thousands_separators_cb (MathEquation          *equation,
			      GParamSpec            *spec G_GNUC_UNUSED,
			      MathPreferencesDialog *dialog)
{
    ctk_toggle_button_set_active(CTK_TOGGLE_BUTTON(ctk_builder_get_object(dialog->priv->ui, "thousands_separator_check")),
                                 math_equation_get_show_thousands_separators(equation));
    g_settings_set_boolean(g_settings_var, "show-thousands", math_equation_get_show_thousands_separators(equation));
}


static void
show_trailing_zeroes_cb (MathEquation          *equation,
			 GParamSpec            *spec G_GNUC_UNUSED,
			 MathPreferencesDialog *dialog)
{
    ctk_toggle_button_set_active(CTK_TOGGLE_BUTTON(ctk_builder_get_object(dialog->priv->ui, "trailing_zeroes_check")),
                                 math_equation_get_show_trailing_zeroes(equation));
    g_settings_set_boolean(g_settings_var, "show-zeroes", math_equation_get_show_trailing_zeroes(equation));
}


static void
number_format_cb (MathEquation          *equation,
		  GParamSpec            *spec G_GNUC_UNUSED,
		  MathPreferencesDialog *dialog)
{
    set_combo_box_from_int(GET_WIDGET(dialog->priv->ui, "number_format_combobox"), math_equation_get_number_format(equation));
    g_settings_set_enum(g_settings_var, "number-format", math_equation_get_number_format(equation));
}


static void
word_size_cb (MathEquation          *equation,
	      GParamSpec            *spec G_GNUC_UNUSED,
	      MathPreferencesDialog *dialog)
{
    set_combo_box_from_int(GET_WIDGET(dialog->priv->ui, "word_size_combobox"), math_equation_get_word_size(equation));
    g_settings_set_int(g_settings_var, "word-size", math_equation_get_word_size(equation));
}


static void
angle_unit_cb (MathEquation          *equation,
	       GParamSpec            *spec G_GNUC_UNUSED,
	       MathPreferencesDialog *dialog)
{
    set_combo_box_from_int(GET_WIDGET(dialog->priv->ui, "angle_unit_combobox"), math_equation_get_angle_units(equation));
    g_settings_set_enum(g_settings_var, "angle-units", math_equation_get_angle_units(equation));
}


static void
create_gui(MathPreferencesDialog *dialog)
{
    CtkWidget *widget;
    CtkTreeModel *model;
    CtkTreeIter iter;
    CtkCellRenderer *renderer;
    gchar *string, **tokens;
    GError *error = NULL;
    static gchar *objects[] = { "preferences_table", "angle_unit_model", "number_format_model",
                                "word_size_model", "decimal_places_adjustment", "number_base_model", NULL };

    // FIXME: Handle errors
    dialog->priv->ui = ctk_builder_new();
    ctk_builder_add_objects_from_resource(dialog->priv->ui, UI_DIALOGS_RESOURCE_PATH, objects, &error);
    if (error)
        g_warning("Error loading preferences UI: %s", error->message);
    g_clear_error(&error);

    ctk_window_set_title(CTK_WINDOW(dialog),
                         /* Title of preferences dialog */
                         _("Preferences"));
    ctk_container_set_border_width(CTK_CONTAINER(dialog), 8);
    ctk_dialog_add_button(CTK_DIALOG(dialog),
                          /* Icon name on close button in preferences dialog */
                          "ctk-close", CTK_RESPONSE_CLOSE);

    ctk_window_set_icon_name (CTK_WINDOW(dialog), "accessories-calculator");

    g_signal_connect(dialog, "response", G_CALLBACK(preferences_response_cb), NULL);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(preferences_dialog_delete_cb), NULL);
    ctk_box_pack_start(CTK_BOX(ctk_dialog_get_content_area(CTK_DIALOG(dialog))), GET_WIDGET(dialog->priv->ui, "preferences_table"), TRUE, TRUE, 0);

    widget = GET_WIDGET(dialog->priv->ui, "angle_unit_combobox");
    model = ctk_combo_box_get_model(CTK_COMBO_BOX(widget));
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Preferences dialog: Angle unit combo box: Use degrees for trigonometric calculations */
                       _("Degrees"), 1, MP_DEGREES, -1);
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Preferences dialog: Angle unit combo box: Use radians for trigonometric calculations */
                       _("Radians"), 1, MP_RADIANS, -1);
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Preferences dialog: Angle unit combo box: Use gradians for trigonometric calculations */
                       _("Gradians"), 1, MP_GRADIANS, -1);
    renderer = ctk_cell_renderer_text_new();
    ctk_cell_layout_pack_start(CTK_CELL_LAYOUT(widget), renderer, TRUE);
    ctk_cell_layout_add_attribute(CTK_CELL_LAYOUT(widget), renderer, "text", 0);

    widget = GET_WIDGET(dialog->priv->ui, "number_format_combobox");
    model = ctk_combo_box_get_model(CTK_COMBO_BOX(widget));
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Number display mode combo: Automatic, e.g. 1234 (or scientific for large number 1.234×10^99) */
                       _("Automatic"), 1, MP_DISPLAY_FORMAT_AUTOMATIC, -1);
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Number display mode combo: Fixed, e.g. 1234 */
                       _("Fixed"), 1, MP_DISPLAY_FORMAT_FIXED, -1);
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Number display mode combo: Scientific, e.g. 1.234×10^3 */
                       _("Scientific"), 1, MP_DISPLAY_FORMAT_SCIENTIFIC, -1);
    ctk_list_store_append(CTK_LIST_STORE(model), &iter);
    ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0,
                       /* Number display mode combo: Engineering, e.g. 1.234k */
                       _("Engineering"), 1, MP_DISPLAY_FORMAT_ENGINEERING, -1);
    renderer = ctk_cell_renderer_text_new();
    ctk_cell_layout_pack_start(CTK_CELL_LAYOUT(widget), renderer, TRUE);
    ctk_cell_layout_add_attribute(CTK_CELL_LAYOUT(widget), renderer, "text", 0);

    widget = GET_WIDGET(dialog->priv->ui, "word_size_combobox");
    renderer = ctk_cell_renderer_text_new();
    ctk_cell_layout_pack_start(CTK_CELL_LAYOUT(widget), renderer, TRUE);
    ctk_cell_layout_add_attribute(CTK_CELL_LAYOUT(widget), renderer, "text", 0);

    /* Label used in preferences dialog.  The %d is replaced by a spinbutton */
    string = _("Show %d decimal _places");
    tokens = g_strsplit(string, "%d", 2);
    widget = GET_WIDGET(dialog->priv->ui, "decimal_places_label1");
    if (tokens[0])
        string = g_strstrip(tokens[0]);
    else
        string = "";
    if (string[0] != '\0')
        ctk_label_set_text_with_mnemonic(CTK_LABEL(widget), string);
    else
        ctk_widget_hide(widget);

    widget = GET_WIDGET(dialog->priv->ui, "decimal_places_label2");
    if (tokens[0] && tokens[1])
        string = g_strstrip(tokens[1]);
    else
        string = "";
    if (string[0] != '\0')
        ctk_label_set_text_with_mnemonic(CTK_LABEL(widget), string);
    else
        ctk_widget_hide(widget);

    g_strfreev(tokens);

    ctk_builder_connect_signals(dialog->priv->ui, dialog);

    g_signal_connect(dialog->priv->equation, "notify::accuracy", G_CALLBACK(accuracy_cb), dialog);
    g_signal_connect(dialog->priv->equation, "notify::show-thousands-separators", G_CALLBACK(show_thousands_separators_cb), dialog);
    g_signal_connect(dialog->priv->equation, "notify::show-trailing_zeroes", G_CALLBACK(show_trailing_zeroes_cb), dialog);
    g_signal_connect(dialog->priv->equation, "notify::number-format", G_CALLBACK(number_format_cb), dialog);
    g_signal_connect(dialog->priv->equation, "notify::word-size", G_CALLBACK(word_size_cb), dialog);
    g_signal_connect(dialog->priv->equation, "notify::angle-units", G_CALLBACK(angle_unit_cb), dialog);

    accuracy_cb(dialog->priv->equation, NULL, dialog);
    show_thousands_separators_cb(dialog->priv->equation, NULL, dialog);
    show_trailing_zeroes_cb(dialog->priv->equation, NULL, dialog);
    number_format_cb(dialog->priv->equation, NULL, dialog);
    word_size_cb(dialog->priv->equation, NULL, dialog);
    angle_unit_cb(dialog->priv->equation, NULL, dialog);
}


static void
math_preferences_set_property(GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    MathPreferencesDialog *self;

    self = MATH_PREFERENCES(object);

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
math_preferences_get_property(GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    MathPreferencesDialog *self;

    self = MATH_PREFERENCES(object);

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
math_preferences_class_init(MathPreferencesDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = math_preferences_get_property;
    object_class->set_property = math_preferences_set_property;

    g_object_class_install_property(object_class,
                                    PROP_EQUATION,
                                    g_param_spec_object("equation",
                                                        "equation",
                                                        "Equation being configured",
                                                        math_equation_get_type(),
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
math_preferences_init(MathPreferencesDialog *dialog)
{
    dialog->priv = math_preferences_get_instance_private (dialog);
}
