/*
 * Copyright (C) 2008-2011 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#ifndef MATH_BUTTONS_H
#define MATH_BUTTONS_H

#include <glib-object.h>
#include <ctk/ctk.h>
#include "math-equation.h"

G_BEGIN_DECLS

#define MATH_BUTTONS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), math_buttons_get_type(), MathButtons))

typedef struct MathButtonsPrivate MathButtonsPrivate;

typedef struct
{
    CtkVBox parent_instance;
    MathButtonsPrivate *priv;
} MathButtons;

typedef struct
{
    CtkVBoxClass parent_class;
} MathButtonsClass;

typedef enum {
    BASIC,
    ADVANCED,
    FINANCIAL,
    PROGRAMMING
} ButtonMode;

GType math_buttons_get_type(void);

MathButtons *math_buttons_new(MathEquation *equation);

void math_buttons_set_mode(MathButtons *buttons, ButtonMode mode);

ButtonMode math_buttons_get_mode(MathButtons *buttons);

void math_buttons_set_programming_base(MathButtons *buttons, gint base);

gint math_buttons_get_programming_base(MathButtons *buttons);

void exponent_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void subtract_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void button_cb (CtkWidget *widget, MathButtons *buttons);

void solve_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void clear_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void delete_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void undo_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void memory_cb (CtkWidget *widget, MathButtons *buttons);

void shift_left_cb (CtkWidget *widget, MathButtons *buttons);

void shift_right_cb (CtkWidget *widget, MathButtons *buttons);

void function_cb (CtkWidget *widget, MathButtons *buttons);

void const_cb (CtkWidget *widget, MathButtons *buttons);

void factorize_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void digit_cb (CtkWidget *widget, MathButtons *buttons);

void numeric_point_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void finc_cb (CtkWidget *widget, MathButtons *buttons);

void insert_character_code_cb (CtkWidget *widget G_GNUC_UNUSED, MathButtons *buttons);

void finc_activate_cb (CtkWidget *widget, MathButtons *buttons);

void finc_response_cb (CtkWidget *widget, gint response_id, MathButtons *buttons);

void character_code_dialog_response_cb (CtkWidget *dialog, gint response_id, MathButtons *buttons);

void character_code_dialog_activate_cb (CtkWidget *entry G_GNUC_UNUSED, MathButtons *buttons);

gboolean character_code_dialog_delete_cb (CtkWidget *dialog, CdkEvent *event G_GNUC_UNUSED, MathButtons *buttons);

gboolean bit_toggle_cb (CtkWidget *event_box, CdkEventButton *event G_GNUC_UNUSED, MathButtons *buttons);

void set_superscript_cb (CtkWidget *widget, MathButtons *buttons);

void set_subscript_cb (CtkWidget *widget, MathButtons *buttons);

G_END_DECLS

#endif /* MATH_BUTTONS_H */
