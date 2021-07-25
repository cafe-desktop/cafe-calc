/*
 * Copyright (C) 2008-2011 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <string.h>
#include <glib/gi18n.h>
#include <cdk/cdkkeysyms.h>

#include "math-display.h"

enum {
    PROP_0,
    PROP_EQUATION
};

struct MathDisplayPrivate
{
    /* Equation being displayed */
    MathEquation *equation;

    /* Display widget */
    CtkWidget *text_view;

    /* Buffer that shows errors etc */
    CtkTextBuffer *info_buffer;

    /* Spinner widget that shows if we're calculating a response */
    CtkWidget *spinner;
};

G_DEFINE_TYPE_WITH_PRIVATE (MathDisplay, math_display, CTK_TYPE_VIEWPORT);

#define GET_WIDGET(ui, name)  CTK_WIDGET(ctk_builder_get_object(ui, name))

MathDisplay *
math_display_new()
{
    return g_object_new(math_display_get_type(), "equation", math_equation_new(), NULL);
}


MathDisplay *
math_display_new_with_equation(MathEquation *equation)
{
    return g_object_new(math_display_get_type(), "equation", equation, NULL);
}


MathEquation *
math_display_get_equation(MathDisplay *display)
{
    return display->priv->equation;
}


static gboolean
display_key_press_cb(CtkWidget *widget, GdkEventKey *event, MathDisplay *display)
{
    int state;
    guint32 c;
    guint new_keyval = 0;

    /* Treat keypad keys as numbers even when numlock is off */
    switch(event->keyval)
    {
    case GDK_KEY_KP_Delete: /* Period without numlock */
        new_keyval = GDK_KEY_KP_Decimal;
        break;
    case GDK_KEY_KP_Insert:
        new_keyval = GDK_KEY_0;
        break;
    case GDK_KEY_KP_End:
        new_keyval = GDK_KEY_1;
        break;
    case GDK_KEY_KP_Down:
        new_keyval = GDK_KEY_2;
        break;
    case GDK_KEY_KP_Page_Down:
        new_keyval = GDK_KEY_3;
        break;
    case GDK_KEY_KP_Left:
        new_keyval = GDK_KEY_4;
        break;
    case GDK_KEY_KP_Begin: /* This is apparently what "5" does when numlock is off. */
        new_keyval = GDK_KEY_5;
        break;
    case GDK_KEY_KP_Right:
        new_keyval = GDK_KEY_6;
        break;
    case GDK_KEY_KP_Home:
        new_keyval = GDK_KEY_7;
        break;
    case GDK_KEY_KP_Up:
        new_keyval = GDK_KEY_8;
        break;
    case GDK_KEY_KP_Page_Up:
        new_keyval = GDK_KEY_9;
        break;
    }

    if (new_keyval) {
        gboolean result;
        GdkEvent *new_event;

        new_event = cdk_event_copy((GdkEvent *)event);
        ((GdkEventKey *)new_event)->keyval = new_keyval;
        g_signal_emit_by_name(widget, "key-press-event", new_event, &result);
        cdk_event_free(new_event);
        return result;
    }

    state = event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK);
    c = cdk_keyval_to_unicode(event->keyval);

    /* Solve on enter */
    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        math_equation_solve(display->priv->equation);
        return TRUE;
    }

    /* Clear on escape */
    if ((event->keyval == GDK_KEY_Escape && state == 0) ||
        (event->keyval == GDK_KEY_BackSpace && state == GDK_CONTROL_MASK) ||
        (event->keyval == GDK_KEY_Delete && state == GDK_SHIFT_MASK)) {
        math_equation_clear(display->priv->equation);
        return TRUE;
    }

    /* Numeric keypad will often insert '.' regardless of locale */
    if (event->keyval == GDK_KEY_KP_Decimal) {
        math_equation_insert_numeric_point(display->priv->equation);
        return TRUE;
    }

    /* Substitute */
    if (state == 0) {
        if (c == '*') {
            math_equation_insert(display->priv->equation, "×");
            return TRUE;
        }
        if (c == '/') {
            math_equation_insert(display->priv->equation, "÷");
            return TRUE;
        }
        if (c == '-') {
            math_equation_insert_subtract(display->priv->equation);
            return TRUE;
        }
    }

    /* Shortcuts */
    if (state == GDK_CONTROL_MASK) {
        switch(event->keyval)
        {
        case GDK_KEY_bracketleft:
            math_equation_insert(display->priv->equation, "⌈");
            return TRUE;
        case GDK_KEY_bracketright:
            math_equation_insert(display->priv->equation, "⌉");
            return TRUE;
        case GDK_KEY_e:
            math_equation_insert_exponent(display->priv->equation);
            return TRUE;
        case GDK_KEY_f:
            math_equation_factorize(display->priv->equation);
            return TRUE;
        case GDK_KEY_i:
            math_equation_insert(display->priv->equation, "⁻¹");
            return TRUE;
        case GDK_KEY_p:
            math_equation_insert(display->priv->equation, "π");
            return TRUE;
        case GDK_KEY_r:
            math_equation_insert(display->priv->equation, "√");
            return TRUE;
        case GDK_KEY_u:
            math_equation_insert(display->priv->equation, "µ");
            return TRUE;
        case GDK_KEY_minus:
             math_equation_insert(display->priv->equation, "⁻");
             return TRUE;
        case GDK_KEY_apostrophe:
             math_equation_insert(display->priv->equation, "°");
             return TRUE;
        }
    }
    if (state == GDK_MOD1_MASK) {
        switch(event->keyval)
        {
        case GDK_KEY_bracketleft:
            math_equation_insert(display->priv->equation, "⌊");
            return TRUE;
        case GDK_KEY_bracketright:
            math_equation_insert(display->priv->equation, "⌋");
            return TRUE;
        }
    }

    if (state == GDK_CONTROL_MASK || math_equation_get_number_mode(display->priv->equation) == SUPERSCRIPT) {
        switch(event->keyval)
        {
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            math_equation_insert(display->priv->equation, "⁰");
            return TRUE;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            math_equation_insert(display->priv->equation, "¹");
            return TRUE;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            math_equation_insert(display->priv->equation, "²");
            return TRUE;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            math_equation_insert(display->priv->equation, "³");
            return TRUE;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            math_equation_insert(display->priv->equation, "⁴");
            return TRUE;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            math_equation_insert(display->priv->equation, "⁵");
            return TRUE;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            math_equation_insert(display->priv->equation, "⁶");
            return TRUE;
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            math_equation_insert(display->priv->equation, "⁷");
            return TRUE;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            math_equation_insert(display->priv->equation, "⁸");
            return TRUE;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            math_equation_insert(display->priv->equation, "⁹");
            return TRUE;
        }
    }
    else if (state == GDK_MOD1_MASK || math_equation_get_number_mode(display->priv->equation) == SUBSCRIPT) {
        switch(event->keyval)
        {
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            math_equation_insert(display->priv->equation, "₀");
            return TRUE;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            math_equation_insert(display->priv->equation, "₁");
            return TRUE;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            math_equation_insert(display->priv->equation, "₂");
            return TRUE;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            math_equation_insert(display->priv->equation, "₃");
            return TRUE;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            math_equation_insert(display->priv->equation, "₄");
            return TRUE;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            math_equation_insert(display->priv->equation, "₅");
            return TRUE;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            math_equation_insert(display->priv->equation, "₆");
            return TRUE;
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            math_equation_insert(display->priv->equation, "₇");
            return TRUE;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            math_equation_insert(display->priv->equation, "₈");
            return TRUE;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            math_equation_insert(display->priv->equation, "₉");
            return TRUE;
        }
    }

    return FALSE;
}


static gboolean
key_press_cb(MathDisplay *display, GdkEventKey *event)
{
    gboolean result;
    g_signal_emit_by_name(display->priv->text_view, "key-press-event", event, &result);
    return result;
}


static void
status_changed_cb(MathEquation *equation, GParamSpec *spec, MathDisplay *display)
{
    ctk_text_buffer_set_text(display->priv->info_buffer, math_equation_get_status(equation), -1);
    if (math_equation_in_solve(equation) && !ctk_widget_get_visible(display->priv->spinner)) {
        ctk_widget_show(display->priv->spinner);
        ctk_spinner_start(CTK_SPINNER(display->priv->spinner));
    }
    else if (!math_equation_in_solve(equation) && ctk_widget_get_visible(display->priv->spinner)) {
        ctk_widget_hide(display->priv->spinner);
        ctk_spinner_stop(CTK_SPINNER(display->priv->spinner));
    }
}

static void _text_view_override_font (CtkWidget *widget, PangoFontDescription *font)
{
    CtkCssProvider *provider;
    gchar          *css;
    gchar          *family;
    gchar          *weight;
    gchar          *size;
    const gchar    *style;

    family = g_strdup_printf ("font-family: %s;", pango_font_description_get_family (font));
    weight = g_strdup_printf ("font-weight: %d;", pango_font_description_get_weight (font));
    if (pango_font_description_get_style (font) == PANGO_STYLE_NORMAL)
        style = "font-style: normal;";
    else if (pango_font_description_get_style (font) == PANGO_STYLE_ITALIC)
        style = "font-style: italic;";
    else
        style = "font-style: oblique;";

    size = g_strdup_printf ("font-size: %d%s;",
                             pango_font_description_get_size (font) / PANGO_SCALE,
                             pango_font_description_get_size_is_absolute (font) ? "px" : "pt");
    css = g_strdup_printf ("textview { %s %s %s %s }", family, weight, style, size);

    provider = ctk_css_provider_new ();
    ctk_css_provider_load_from_data (provider, css, -1, NULL);
    g_free (css);
    g_free (family);
    g_free (weight);
    g_free (size);

    ctk_style_context_add_provider (ctk_widget_get_style_context (widget),
                                    CTK_STYLE_PROVIDER (provider),
                                    CTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);
}

static void
create_gui(MathDisplay *display)
{
    CtkWidget *info_view, *info_box, *main_box;
    PangoFontDescription *font_desc;
    CtkStyleContext *context;
    CtkStateFlags state;

    main_box = ctk_box_new(CTK_ORIENTATION_VERTICAL, 0);
    ctk_container_add(CTK_CONTAINER(display), main_box);

    g_signal_connect(display, "key-press-event", G_CALLBACK(key_press_cb), display);

    display->priv->text_view = ctk_text_view_new_with_buffer(CTK_TEXT_BUFFER(display->priv->equation));
    ctk_text_view_set_wrap_mode(CTK_TEXT_VIEW(display->priv->text_view), CTK_WRAP_WORD);
    ctk_text_view_set_accepts_tab(CTK_TEXT_VIEW(display->priv->text_view), FALSE);
    ctk_text_view_set_pixels_above_lines(CTK_TEXT_VIEW(display->priv->text_view), 8);
    ctk_text_view_set_pixels_below_lines(CTK_TEXT_VIEW(display->priv->text_view), 2);
    /* TEMP: Disabled for now as CTK+ doesn't properly render a right aligned right margin, see bug #482688 */
    /*ctk_text_view_set_right_margin(CTK_TEXT_VIEW(display->priv->text_view), 6);*/
    ctk_text_view_set_justification(CTK_TEXT_VIEW(display->priv->text_view), CTK_JUSTIFY_RIGHT);
    context = ctk_widget_get_style_context (display->priv->text_view);
    state = ctk_widget_get_state_flags (CTK_WIDGET (display->priv->text_view));
    ctk_style_context_get (context, state, CTK_STYLE_PROPERTY_FONT, &font_desc, NULL);
    pango_font_description_set_size(font_desc, 16 * PANGO_SCALE);
    _text_view_override_font (display->priv->text_view, font_desc);
    pango_font_description_free(font_desc);
    ctk_widget_set_name(display->priv->text_view, "displayitem");
    atk_object_set_role(ctk_widget_get_accessible(display->priv->text_view), ATK_ROLE_EDITBAR);
  //FIXME:<property name="AtkObject::accessible-description" translatable="yes" comments="Accessible description for the area in which results are displayed">Result Region</property>
    g_signal_connect(display->priv->text_view, "key-press-event", G_CALLBACK(display_key_press_cb), display);
    ctk_box_pack_start(CTK_BOX(main_box), display->priv->text_view, TRUE, TRUE, 0);

    info_box = ctk_box_new(CTK_ORIENTATION_HORIZONTAL, 6);
    ctk_box_pack_start(CTK_BOX(main_box), info_box, FALSE, TRUE, 0);

    info_view = ctk_text_view_new();
    ctk_text_view_set_wrap_mode(CTK_TEXT_VIEW(info_view), CTK_WRAP_WORD);
    ctk_widget_set_can_focus(info_view, TRUE); // FIXME: This should be FALSE but it locks the cursor inside the main view for some reason
    ctk_text_view_set_cursor_visible(CTK_TEXT_VIEW(info_view), FALSE); // FIXME: Just here so when incorrectly gets focus doesn't look editable
    ctk_text_view_set_editable(CTK_TEXT_VIEW(info_view), FALSE);
    ctk_text_view_set_justification(CTK_TEXT_VIEW(info_view), CTK_JUSTIFY_RIGHT);
    /* TEMP: Disabled for now as CTK+ doesn't properly render a right aligned right margin, see bug #482688 */
    /*ctk_text_view_set_right_margin(CTK_TEXT_VIEW(info_view), 6);*/
    ctk_box_pack_start(CTK_BOX(info_box), info_view, TRUE, TRUE, 0);
    display->priv->info_buffer = ctk_text_view_get_buffer(CTK_TEXT_VIEW(info_view));

    display->priv->spinner = ctk_spinner_new();
    ctk_box_pack_end(CTK_BOX(info_box), display->priv->spinner, FALSE, FALSE, 0);

    ctk_widget_show(info_box);
    ctk_widget_show(info_view);
    ctk_widget_show(display->priv->text_view);
    ctk_widget_show(main_box);

    g_signal_connect(display->priv->equation, "notify::status", G_CALLBACK(status_changed_cb), display);
    status_changed_cb(display->priv->equation, NULL, display);
}


static void
math_display_set_property(GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    MathDisplay *self;

    self = MATH_DISPLAY(object);

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
math_display_get_property(GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    MathDisplay *self;

    self = MATH_DISPLAY(object);

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
math_display_class_init(MathDisplayClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = math_display_get_property;
    object_class->set_property = math_display_set_property;

    g_object_class_install_property(object_class,
                                    PROP_EQUATION,
                                    g_param_spec_object("equation",
                                                        "equation",
                                                        "Equation being displayed",
                                                        math_equation_get_type(),
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
math_display_init(MathDisplay *display)
{
    display->priv = math_display_get_instance_private (display);
}
