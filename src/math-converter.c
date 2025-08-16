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

#include "math-converter.h"
#include "unit-manager.h"
#include "currency-manager.h"

enum {
    CHANGED,
    LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0, };

struct MathConverterPrivate
{
    MathEquation *equation;

    gchar *category;

    CtkWidget *from_combo;
    CtkWidget *to_combo;

    CtkWidget *result_label;
};


G_DEFINE_TYPE_WITH_PRIVATE (MathConverter, math_converter, CTK_TYPE_BOX);

static void display_changed_cb(MathEquation *equation, GParamSpec *spec, MathConverter *converter);
static void update_from_model(MathConverter *converter);


MathConverter *
math_converter_new(MathEquation *equation)
{
    MathConverter *converter = g_object_new(math_converter_get_type(), NULL);
    converter->priv->equation = g_object_ref(equation);
    g_signal_connect(converter->priv->equation, "notify::display", G_CALLBACK(display_changed_cb), converter);
    update_from_model(converter);
    return converter;
}


static gboolean
convert_equation(MathConverter *converter, const MPNumber *x, MPNumber *z)
{
    CtkTreeIter from_iter, to_iter;
    UnitCategory *category = NULL;
    Unit *source_unit = NULL, *target_unit = NULL;
    gboolean result;

    if (!ctk_combo_box_get_active_iter(CTK_COMBO_BOX(converter->priv->from_combo), &from_iter) ||
        !ctk_combo_box_get_active_iter(CTK_COMBO_BOX(converter->priv->to_combo), &to_iter))
        return FALSE;

    ctk_tree_model_get(ctk_combo_box_get_model(CTK_COMBO_BOX(converter->priv->from_combo)), &from_iter, 1, &category, 2, &source_unit, -1);
    ctk_tree_model_get(ctk_combo_box_get_model(CTK_COMBO_BOX(converter->priv->to_combo)), &to_iter, 2, &target_unit, -1);

    result = unit_category_convert(category, x, source_unit, target_unit, z);

    if (category)
        g_object_unref(category);
    if (source_unit)
        g_object_unref(source_unit);
    if (target_unit)
        g_object_unref(target_unit);

    return result;
}


static void
update_result_label(MathConverter *converter)
{
    MPNumber x, z;
    gboolean enabled;

    if (!converter->priv->result_label)
        return;

    if (math_equation_get_number(converter->priv->equation, &x))
        enabled = convert_equation(converter, &x, &z);
    else
        enabled = FALSE;

    ctk_widget_set_sensitive(converter->priv->result_label, enabled);
    if (enabled) {
        gchar *source_text, *target_text, *label;
        Unit *source_unit, *target_unit;

        math_converter_get_conversion(converter, &source_unit, &target_unit);

        source_text = unit_format(source_unit, &x);
        target_text = unit_format(target_unit, &z);
        label = g_strdup_printf("%s = %s", source_text, target_text);
        ctk_label_set_text(CTK_LABEL(converter->priv->result_label), label);

        g_free(source_text);
        g_free(target_text);
        g_free(label);

        g_object_unref(source_unit);
        g_object_unref(target_unit);
    }
}


static void
display_changed_cb (MathEquation  *equation G_GNUC_UNUSED,
		    GParamSpec    *spec G_GNUC_UNUSED,
		    MathConverter *converter)
{
    update_result_label(converter);
}


static void
update_from_model(MathConverter *converter)
{
    CtkTreeStore *from_model;

    from_model = ctk_tree_store_new(3, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_OBJECT);

    if (converter->priv->category == NULL) {
        const GList *categories, *iter;

        categories = unit_manager_get_categories(unit_manager_get_default());
        for (iter = categories; iter; iter = iter->next) {
            UnitCategory *category = iter->data;
            CtkTreeIter parent;
            const GList *unit_iter;

            ctk_tree_store_append(from_model, &parent, NULL);
            ctk_tree_store_set(from_model, &parent, 0, unit_category_get_display_name(category), 1, category, -1);

            for (unit_iter = unit_category_get_units(category); unit_iter; unit_iter = unit_iter->next) {
                Unit *unit = unit_iter->data;
                CtkTreeIter iter;

                ctk_tree_store_append(from_model, &iter, &parent);
                ctk_tree_store_set(from_model, &iter, 0, unit_get_display_name(unit), 1, category, 2, unit, -1);
            }
        }
    }
    else {
        UnitCategory *category;
        const GList *unit_iter;

        category = unit_manager_get_category(unit_manager_get_default(), converter->priv->category);
        for (unit_iter = unit_category_get_units(category); unit_iter; unit_iter = unit_iter->next) {
            Unit *unit = unit_iter->data;
            CtkTreeIter iter;

            ctk_tree_store_append(from_model, &iter, NULL);
            ctk_tree_store_set(from_model, &iter, 0, unit_get_display_name(unit), 1, category, 2, unit, -1);
        }
    }

    ctk_combo_box_set_model(CTK_COMBO_BOX(converter->priv->from_combo), CTK_TREE_MODEL(from_model));
}


void
math_converter_set_category(MathConverter *converter, const gchar *category)
{
    g_return_if_fail (converter != NULL);

    if (category == NULL && converter->priv->category == NULL)
        return;
    if (category != NULL && converter->priv->category != NULL && strcmp(category, converter->priv->category) == 0)
        return;

    g_free(converter->priv->category);
    converter->priv->category = g_strdup(category);

    update_from_model(converter);
}


const gchar *
math_converter_get_category(MathConverter *converter)
{
    g_return_val_if_fail (converter != NULL, NULL);
    return converter->priv->category;
}


static gboolean
iter_is_unit(CtkTreeModel *model, CtkTreeIter *iter, Unit *unit)
{
    Unit *u;

    ctk_tree_model_get(model, iter, 2, &u, -1);

    if (!u)
        return FALSE;

    g_object_unref(u);
    if (u == unit)
        return TRUE;

    return FALSE;
}


static gboolean
set_active_unit(CtkComboBox *combo, CtkTreeIter *iter, Unit *unit)
{
    CtkTreeModel *model;
    CtkTreeIter child_iter;

    model = ctk_combo_box_get_model(combo);

    if (iter && iter_is_unit(model, iter, unit)) {
        ctk_combo_box_set_active_iter(combo, iter);
        return TRUE;
    }

    if (!ctk_tree_model_iter_children(model, &child_iter, iter))
        return FALSE;

    do {
        if (set_active_unit(combo, &child_iter, unit))
            return TRUE;
    } while (ctk_tree_model_iter_next(model, &child_iter));

    return FALSE;
}


void
math_converter_set_conversion(MathConverter *converter, /*const gchar *category,*/ const gchar *unit_a, const gchar *unit_b)
{
    Unit *ua;
    Unit *ub;

    g_return_if_fail (converter != NULL);
    g_return_if_fail (unit_a != NULL);
    g_return_if_fail (unit_b != NULL);

    ua = unit_manager_get_unit_by_name(unit_manager_get_default(), unit_a);
    ub = unit_manager_get_unit_by_name(unit_manager_get_default(), unit_b);
    if (!ua || !ub)
    {
        CtkTreeModel *model;
        CtkTreeIter iter;

        /* Select the first unit */
        model = ctk_combo_box_get_model(CTK_COMBO_BOX(converter->priv->from_combo));
        if (ctk_tree_model_get_iter_first(model, &iter)) {
            CtkTreeIter child_iter;
            while (ctk_tree_model_iter_children(model, &child_iter, &iter))
                iter = child_iter;
            ctk_combo_box_set_active_iter(CTK_COMBO_BOX(converter->priv->from_combo), &iter);
        }
        return;
    }

    set_active_unit(CTK_COMBO_BOX(converter->priv->from_combo), NULL, ua);
    set_active_unit(CTK_COMBO_BOX(converter->priv->to_combo), NULL, ub);
}


void
math_converter_get_conversion(MathConverter *converter, Unit **from_unit, Unit **to_unit)
{
    CtkTreeIter from_iter, to_iter;

    g_return_if_fail (converter != NULL);
    g_return_if_fail (from_unit != NULL);
    g_return_if_fail (to_unit != NULL);

    ctk_combo_box_get_active_iter(CTK_COMBO_BOX(converter->priv->from_combo), &from_iter);
    ctk_combo_box_get_active_iter(CTK_COMBO_BOX(converter->priv->to_combo), &to_iter);

    ctk_tree_model_get(ctk_combo_box_get_model(CTK_COMBO_BOX(converter->priv->from_combo)), &from_iter, 2, from_unit, -1);
    ctk_tree_model_get(ctk_combo_box_get_model(CTK_COMBO_BOX(converter->priv->to_combo)), &to_iter, 2, to_unit, -1);
}


static void
math_converter_class_init(MathConverterClass *klass)
{
    signals[CHANGED] =
        g_signal_new("changed",
                     G_TYPE_FROM_CLASS (klass),
                     G_SIGNAL_RUN_LAST,
                     G_STRUCT_OFFSET (MathConverterClass, changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);
}


static void
from_combobox_changed_cb(CtkWidget *combo, MathConverter *converter)
{
    CtkTreeModel *model;
    CtkTreeIter iter;
    UnitCategory *category;
    Unit *unit;
    const GList *unit_iter;

    model = ctk_combo_box_get_model(CTK_COMBO_BOX(combo));
    if (!ctk_combo_box_get_active_iter(CTK_COMBO_BOX(combo), &iter))
        return;
    ctk_tree_model_get(model, &iter, 1, &category, 2, &unit, -1);

    /* Set the to combobox to be the list of units can be converted to */
    model = CTK_TREE_MODEL(ctk_list_store_new(3, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_OBJECT));
    for (unit_iter = unit_category_get_units(category); unit_iter; unit_iter = unit_iter->next) {
        Unit *u = unit_iter->data;
        if (u == unit)
            continue;
        ctk_list_store_append(CTK_LIST_STORE(model), &iter);
        ctk_list_store_set(CTK_LIST_STORE(model), &iter, 0, unit_get_display_name(u), 1, category, 2, u, -1);
    }
    ctk_combo_box_set_model(CTK_COMBO_BOX(converter->priv->to_combo), model);

    /* Select the first possible unit */
    ctk_combo_box_set_active(CTK_COMBO_BOX(converter->priv->to_combo), 0);

    g_object_unref(category);
    g_object_unref(unit);
}


static void
to_combobox_changed_cb (CtkWidget     *combo G_GNUC_UNUSED,
			MathConverter *converter)
{
    /* Conversion must have changed */
    update_result_label(converter);

    g_signal_emit(converter, signals[CHANGED], 0);
}


static void
from_cell_data_func(CtkCellLayout   *cell_layout G_GNUC_UNUSED,
		    CtkCellRenderer *cell,
		    CtkTreeModel    *tree_model,
		    CtkTreeIter     *iter,
		    gpointer         data G_GNUC_UNUSED)
{
    g_object_set(cell, "sensitive", !ctk_tree_model_iter_has_child(tree_model, iter), NULL);
}


static void
currency_updated_cb (CurrencyManager *manager G_GNUC_UNUSED,
		     MathConverter   *converter)
{
    update_result_label(converter);
}

static void
swap_button_clicked_cb (CtkButton     *button G_GNUC_UNUSED,
			MathConverter *converter)
{
    Unit *from_unit, *to_unit;
    MPNumber x, z;

    if (math_equation_get_number(converter->priv->equation, &x) &&
        convert_equation(converter, &x, &z))
        math_equation_set_number(converter->priv->equation, &z);

    math_converter_get_conversion(converter, &from_unit, &to_unit);
    set_active_unit(CTK_COMBO_BOX(converter->priv->from_combo), NULL, to_unit);
    set_active_unit(CTK_COMBO_BOX(converter->priv->to_combo), NULL, from_unit);

    update_result_label(converter);

    g_object_unref(from_unit);
    g_object_unref(to_unit);
}

static void
math_converter_init(MathConverter *converter)
{
    CtkWidget *hbox, *label, *swap_button;
    CtkCellRenderer *renderer;

    converter->priv = math_converter_get_instance_private (converter);

    ctk_box_set_spacing(CTK_BOX(converter), 6);
    ctk_orientable_set_orientation (CTK_ORIENTABLE (converter),
                                    CTK_ORIENTATION_HORIZONTAL);

    hbox = ctk_box_new(CTK_ORIENTATION_HORIZONTAL, 0);
    ctk_widget_show(hbox);
    ctk_box_pack_start(CTK_BOX(converter), hbox, FALSE, TRUE, 0);

    converter->priv->from_combo = ctk_combo_box_new ();

    renderer = ctk_cell_renderer_text_new();
    ctk_cell_layout_pack_start(CTK_CELL_LAYOUT(converter->priv->from_combo), renderer, TRUE);
    ctk_cell_layout_add_attribute(CTK_CELL_LAYOUT(converter->priv->from_combo), renderer, "text", 0);
    ctk_cell_layout_set_cell_data_func(CTK_CELL_LAYOUT(converter->priv->from_combo),
                                       renderer,
                                       from_cell_data_func,
                                       NULL, NULL);
    g_signal_connect(converter->priv->from_combo, "changed", G_CALLBACK(from_combobox_changed_cb), converter);
    ctk_widget_show(converter->priv->from_combo);
    ctk_box_pack_start(CTK_BOX(hbox), converter->priv->from_combo, FALSE, TRUE, 0);

    label = ctk_label_new(/* Label that is displayed between the two conversion combo boxes, e.g. "[degrees] in [radians]" */
                          _(" in "));
    ctk_widget_show(label);
    ctk_box_pack_start(CTK_BOX(hbox), label, FALSE, TRUE, 5);

    converter->priv->to_combo = ctk_combo_box_new();
    renderer = ctk_cell_renderer_text_new();
    ctk_cell_layout_pack_start(CTK_CELL_LAYOUT(converter->priv->to_combo), renderer, TRUE);
    ctk_cell_layout_add_attribute(CTK_CELL_LAYOUT(converter->priv->to_combo), renderer, "text", 0);
    g_signal_connect(converter->priv->to_combo, "changed", G_CALLBACK(to_combobox_changed_cb), converter);
    ctk_widget_show(converter->priv->to_combo);
    ctk_box_pack_start(CTK_BOX(hbox), converter->priv->to_combo, FALSE, TRUE, 0);

    swap_button = ctk_button_new_with_label ("⇆");
    ctk_widget_set_tooltip_text (swap_button,
                                 /* Tooltip for swap conversion button */
                                 _("Switch conversion units"));
    ctk_button_set_relief (CTK_BUTTON (swap_button), CTK_RELIEF_NONE);
    g_signal_connect (swap_button, "clicked", G_CALLBACK (swap_button_clicked_cb), converter);
    ctk_widget_show(swap_button);
    ctk_box_pack_start(CTK_BOX(hbox), swap_button, FALSE, TRUE, 0);

    converter->priv->result_label = ctk_label_new("");
    ctk_widget_set_halign (converter->priv->result_label, CTK_ALIGN_END);
    ctk_widget_set_sensitive(converter->priv->result_label, FALSE);
    ctk_widget_show(converter->priv->result_label);
    ctk_box_pack_start(CTK_BOX(converter), converter->priv->result_label, TRUE, TRUE, 0);

    g_signal_connect(currency_manager_get_default(), "updated", G_CALLBACK(currency_updated_cb), converter);
}
