/*
 * Copyright (C) 2008-2011 Robert Ancell.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>

#include "mp-equation.h"
#include "mp-serializer.h"
#include "unit-manager.h"

static MPEquationOptions options;

static int fails = 0;
static int passes = 0;

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

#define test(X, Y, Z) Test(X, Y, Z, 9)

static void pass(const char *format, ...) __attribute__((format(printf, 1, 2)));
static void fail(const char *format, ...) __attribute__((format(printf, 1, 2)));


static void pass(const char *format, ...)
{
/*    va_list args;

    printf(" PASS: ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
*/
    passes += 1;
}

static void fail(const char *format, ...)
{
    va_list args;

    printf("*FAIL: ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    fails++;
}


static const char *
error_code_to_string(MPErrorCode error)
{
    static char error_string[1024];

    if (error != PARSER_ERR_MP)
        return mp_error_code_to_string(error);

    snprintf(error_string, 1024, "PARSER_ERR_MP(\"%s\")", mp_get_error());
    return error_string;
}


static void
Test(char *expression, char *expected, int expected_error, int trailing_digits)
{
    MPErrorCode error;
    MPNumber result;

    error = mp_equation_parse(expression, &options, &result, NULL);

    if (error == 0) {
        char *result_str;
        MpSerializer *serializer;

        serializer = mp_serializer_new(MP_DISPLAY_FORMAT_FIXED, options.base, trailing_digits);
        result_str = mp_serializer_to_string(serializer, &result);
        g_object_unref(serializer);

        if(expected_error != PARSER_ERR_NONE)
            fail("'%s' -> %s, expected error %s", expression, result_str, error_code_to_string(expected_error));
        else if(strcmp(result_str, expected) != 0)
            fail("'%s' -> '%s', expected '%s'", expression, result_str, expected);
        else
            pass("'%s' -> '%s'", expression, result_str);
        g_free(result_str);
    }
    else {
        if(error == expected_error)
            pass("'%s' -> error %s", expression, error_code_to_string(error));
        else if(expected_error == PARSER_ERR_NONE)
            fail("'%s' -> error %s, expected result %s", expression,
                 error_code_to_string(error), expected);
        else
            fail("'%s' -> error %s, expected error %s", expression,
                 error_code_to_string(error), error_code_to_string(expected_error));
    }
}


static int
do_convert(const MPNumber *x, const char *x_units, const char *z_units, MPNumber *z, void *data)
{
    return unit_manager_convert_by_symbol(unit_manager_get_default(), x, x_units, z_units, z);
}


static void
test_conversions()
{
    memset(&options, 0, sizeof(options));
    options.base = 10;
    options.wordlen = 32;
    options.angle_units = MP_DEGREES;
    options.convert = do_convert;

    /* Angle units */
    //test("?? radians in degrees", "180", 0);
    test("100 gradians in degrees", "90", 0);

    /* Length */
    test("1 meter in mm", "1000", 0);
    test("1m in mm", "1000", 0);
    test("1 inch in cm", "2.54", 0);

    /* Area */
    test("1m?? in mm??", "1000000", 0);

    /* Volume */
    test("1m?? in mm??", "1000000000", 0);

    /* Weight */
    test("1 kg in pounds", "2.204622622", 0);

    /* Duration */
    test("1 minute in seconds", "60", 0);
    test("1s in ms", "1000", 0);

    /* Temperature */
    //test("100??C in ??F", "", 0);
    //test("0??C in ??F", "32", 0);
    //test("0??K in ??C", "???273.15", 0);
    test("100degC in degF", "212", 0);
    test("0degC in degF", "32", 0);
    test("0 K in degC", "???273.15", 0);
}


static int
variable_is_defined(const char *name, void *data)
{
    return strcmp (name, "x") == 0 || strcmp (name, "y") == 0;
}


static int
get_variable(const char *name, MPNumber *z, void *data)
{
    if (strcmp (name, "x") == 0) {
        mp_set_from_integer (2, z);
        return 1;
    }
    if (strcmp (name, "y") == 0) {
        mp_set_from_integer (3, z);
        return 1;
    }
    return 0;
}


static void
set_variable(const char *name, const MPNumber *x, void *data)
{
}

static void
test_equations()
{
    memset(&options, 0, sizeof(options));
    options.wordlen = 32;
    options.angle_units = MP_DEGREES;
    options.variable_is_defined = variable_is_defined;
    options.get_variable = get_variable;
    options.set_variable = set_variable;

    options.base = 2;
    test("2??????", "10", 0);

    options.base = 8;
    test("16434824??????", "76543210", 0);

    options.base = 16;
    test("FF", "FF", 0);
    test("18364758544493064720??????", "FEDCBA9876543210", 0);

    options.base = 10;
    test("0???", "0", 0); test("0???", "0", 0); test("0", "0", 0); test("0??????", "0", 0);
    test("1???", "1", 0); test("1???", "1", 0); test("1", "1", 0); test("1??????", "1", 0);
    test("2???", "", PARSER_ERR_INVALID); test("2???", "2", 0); test("2", "2", 0); test("2??????", "2", 0);
    test("3???", "", PARSER_ERR_INVALID); test("3???", "3", 0); test("3", "3", 0); test("3??????", "3", 0);
    test("4???", "", PARSER_ERR_INVALID); test("4???", "4", 0); test("4", "4", 0); test("4??????", "4", 0);
    test("5???", "", PARSER_ERR_INVALID); test("5???", "5", 0); test("5", "5", 0); test("5??????", "5", 0);
    test("6???", "", PARSER_ERR_INVALID); test("6???", "6", 0); test("6", "6", 0); test("6??????", "6", 0);
    test("7???", "", PARSER_ERR_INVALID); test("7???", "7", 0); test("7", "7", 0); test("7??????", "7", 0);
    test("8???", "", PARSER_ERR_INVALID); test("8???", "", PARSER_ERR_INVALID); test("8", "8", 0); test("8??????", "8", 0);
    test("9???", "", PARSER_ERR_INVALID); test("9???", "", PARSER_ERR_INVALID); test("9", "9", 0); test("9??????", "9", 0);
    test("A???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("A???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("A", "", PARSER_ERR_UNKNOWN_VARIABLE); test("A??????", "10", 0);
    test("B???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("B???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("B", "", PARSER_ERR_UNKNOWN_VARIABLE); test("B??????", "11", 0);
    test("C???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("C???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("C", "", PARSER_ERR_UNKNOWN_VARIABLE); test("C??????", "12", 0);
    test("D???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("D???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("D", "", PARSER_ERR_UNKNOWN_VARIABLE); test("D??????", "13", 0);
    test("E???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("E???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("E", "", PARSER_ERR_UNKNOWN_VARIABLE); test("E??????", "14", 0);
    test("F???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("F???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("F", "", PARSER_ERR_UNKNOWN_VARIABLE); test("F??????", "15", 0);
    test("a???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("a???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("a", "", PARSER_ERR_UNKNOWN_VARIABLE); test("a??????", "10", 0);
    test("b???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("b???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("b", "", PARSER_ERR_UNKNOWN_VARIABLE); test("b??????", "11", 0);
    test("c???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("c???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("c", "", PARSER_ERR_UNKNOWN_VARIABLE); test("c??????", "12", 0);
    test("d???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("d???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("d", "", PARSER_ERR_UNKNOWN_VARIABLE); test("d??????", "13", 0);
    test("e???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("e???", "", PARSER_ERR_UNKNOWN_VARIABLE); /* e is a built-in variable */              test("e??????", "14", 0);
    test("f???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("f???", "", PARSER_ERR_UNKNOWN_VARIABLE); test("f", "", PARSER_ERR_UNKNOWN_VARIABLE); test("f??????", "15", 0);

    test("+1", "1", 0);
    test("???1", "???1", 0);
    test("+ 1", "1", 0); // FIXME: Should this be allowed?
    test("??? 1", "???1", 0); // FIXME: Should this be allowed?
    test("++1", "1", PARSER_ERR_INVALID);
    test("??????1", "1", 0);
    test("255", "255", 0);
    test("256", "256", 0);
    test("??", "0.5", 0);
    test("1??", "1.5", 0);
    test("0??", "0", 0);
    test("1??", "1", 0);
    test("0??30'", "0.5", 0);
    //test("0??0.1'", "1", 0); // FIXME: Not yet supported
    test("0??0'1\"", "0.000277778", 0);
    test("0??0'0.1\"", "0.000027778", 0);
    test("1.00", "1", 0);
    test("1.01", "1.01", 0);

    test("????????????????????", "1234567890", 0);
    test("????????????????????", "1234567890", 0);

/*
    //test("2A", "2000000000000000", 0);
    test("2T", "2000000000000", 0);
    test("2G", "2000000000", 0);
    test("2M", "2000000", 0);
    test("2k", "2000", 0);
    test("2c", "0.02", 0);
    test("2d", "0.2", 0);
    test("2c", "0.02", 0);
    test("2m", "0.002", 0);
    test("2u", "0.000002", 0);
    test("2??", "0.000002", 0);
    test("2n", "0.000000002", 0);
    //test("2p", "0.000000000002", 0); // FIXME: Need to print out significant figures, not decimal places
    //test("2f", "0.000000000000002", 0); // FIXME: Need to print out significant figures, not decimal places
    //test("2A3", "2300000000000000", 0);
    test("2T3", "2300000000000", 0);
    test("2G3", "2300000000", 0);
    test("2M3", "2300000", 0);
    test("2k3", "2300", 0);
    test("2c3", "0.023", 0);
    test("2d3", "0.23", 0);
    test("2c3", "0.023", 0);
    test("2m3", "0.0023", 0);
    test("2u3", "0.0000023", 0);
    test("2??3", "0.0000023", 0);
    //test("2n3", "0.0000000023", 0); // FIXME: Need to print out significant figures, not decimal places
    //test("2p3", "0.0000000000023", 0); // FIXME: Need to print out significant figures, not decimal places
    //test("2f3", "0.0000000000000023", 0); // FIXME: Need to print out significant figures, not decimal places
*/

    test("2??10^3", "2000", 0);
    test("2??10^???3", "0.002", 0);

    test("x", "2", 0);
    test("y", "3", 0);
    test("z", "", PARSER_ERR_UNKNOWN_VARIABLE);
    test("2y", "6", 0);
    test("y2", "", PARSER_ERR_INVALID);
    test("y 2", "", PARSER_ERR_INVALID);
    test("2z", "", PARSER_ERR_UNKNOWN_VARIABLE);
    test("z2", "", PARSER_ERR_UNKNOWN_VARIABLE);
    test("z 2", "", PARSER_ERR_UNKNOWN_VARIABLE);
    test("z(2)", "", PARSER_ERR_UNKNOWN_VARIABLE);
    test("y??", "9", 0);
    test("2y??", "18", 0);
    test("x??y", "6", 0);
    test("xy", "6", 0);
    test("yx", "6", 0);
    test("2xy", "12", 0);
    test("x??y", "12", 0);
    test("xy??", "18", 0);
    test("(xy)??", "36", 0);
    test("2x??y", "24", 0);
    test("2xy??", "36", 0);
    test("2x??y??", "72", 0);
    test("x??yx??y", "144", 0);
    test("x??+2x?????5", "11", 0);
    test("2(x+3y)", "22", 0);
    test("x(x+3y)", "22", 0);
    test("(x+3y)(2x-4y)", "???88", 0);
    test("2x??+2xy???12y??", "???88", 0);

    test("??", "3.141592654", 0);
    test("pi", "3.141592654", 0);
    test("e", "2.718281828", 0);

    /* Physical constants */

    test("c???", "299792458", 0);
    Test("?????", "0.0000012566370614", 0, 16);
    Test("?????", "0.00000000000885418782", 0, 20);
    Test("G",  "0.0000000000667408", 0, 16);
    Test("h",  "0.000000000000000000000000000000000662607004", 0, 42);
    Test("???", "0.00000000000000000016021766208", 0, 29);
    Test("m???", "0.000000000000000000000000000000910938356", 0, 39);
    Test("m???", "0.000000000000000000000000001672621898", 0, 36);
    test("N???", "602214086000000000000000", 0);

    test("z=99", "99", 0);
    test("longname=99", "99", 0);
    //test("e=99", "", PARSER_ERR_BUILTIN_VARIABLE);

    test("0+0", "0", 0);
    test("1+1", "2", 0);
    test("1+4", "5", 0);
    test("4+1", "5", 0);
    test("40000+0.001", "40000.001", 0);
    test("0.001+40000", "40000.001", 0);
    test("2-3", "???1", 0);
    test("2???3", "???1", 0);
    test("3???2", "1", 0);
    test("40000???0.001", "39999.999", 0);
    test("0.001???40000", "???39999.999", 0);
    test("2*3", "6", 0);
    test("2??3", "6", 0);
    test("???2??3", "???6", 0);
    test("2?????3", "???6", 0);
    test("???2?????3", "6", 0);
    test("6/3", "2", 0);
    test("6??3", "2", 0);
    test("1??2", "0.5", 0);
    test("???6??3", "???2", 0);
    test("6?????3", "???2", 0);
    test("???6?????3", "2", 0);
    test("(???3)??(???6)", "0.5", 0);
    test("2??2", "1", 0);
    test("1203??1", "1203", 0);
    test("???0??32352.689", "0", 0);
    test("1??4", "0.25", 0);
    test("1??3", "0.333333333", 0);
    test("2??3", "0.666666667", 0);
    test("1??0", "", PARSER_ERR_MP);
    test("0??0", "", PARSER_ERR_MP);

    /* Precision */
    test("1000000000000000???1000000000000000", "0", 0);
    test("1000000000000000??1000000000000000", "1", 0);
    test("1000000000000000??0.000000000000001", "1", 0);

    /* Order of operations */
    test("1???0.9???0.1", "0", 0);
    test("1+2??3", "7", 0);
    test("1+(2??3)", "7", 0);
    test("(1+2)??3", "9", 0);
    test("(1+2??3)", "7", 0);
    test("2(1+1)", "4", 0);
    test("4??2(1+1)", "4", 0);
    test ("1 + 2 - 3 * 4 / 5", "0.6", 0);
    test ("20 / 10 mod 3", "2", 0);
    test ("12 / 3 ???4", "8", 0);
    test ("???5!", "10.95445115", 0);
    //test ("4 ^ sin 30", "2", 0);
    test ("4 ^ (sin 30)", "2", 0);
    //test ("4 ^ sin (30)", "2", 0);
    //test ("sin (30) ^ 4", "0.0625", 0);
    //test ("sin 30 ^ 4", "0.0625", 0);
    test ("sin (30 ^ 4)", "0", 0);
    //test ("-ln(1)", "0", 0);
    test ("ln(-1)", "3.141592654i", 0);
    test ("2.33*sqrt(2.5)+6", "9.684053474", 0);

    test ("10 / - 2", "???5", 0);
    test ("10 * - 2", "???20", 0);
    test ("10 ^ -2", "0.01", 0);
    test ("-10 ^ 2", "???100", 0);
    test ("sin (-30)", "???0.5", 0);
    test ("sin - 30", "???0.5", 0);

    test ("6 + 3!", "12", 0);
    test ("4 * 3!", "24", 0);
    test ("100 mod 3!", "4", 0);
    test ("5! mod 7", "1", 0);
    test ("24 / 3!", "4", 0);
    test ("4! / 6", "4", 0);
    test ("cos 5!", "???0.5", 0);
    test ("sin 6!", "0", 0);
    test ("- 4!", "???24", 0);
    test ("3! ^ 3", "216", 0);
    test ("3 ^ 3!", "729", 0);
    //test ("(??????3)^2", "3", 0);

    /* Percentage */
    test("100%", "1", 0);
    test("1%", "0.01", 0);
    test("100+1%", "101", 0);
    test("100???1%", "99", 0);
    test("100??1%", "1", 0);
    test("100??1%", "10000", 0);

    /* Factorial */
    test("0!", "1", 0);
    test("1!", "1", 0);
    test("5!", "120", 0);
    test("69!", "171122452428141311372468338881272839092270544893520369393648040923257279754140647424000000000000000", 0);
    test("0.1!", "", PARSER_ERR_MP);
    test("???1!", "???1", 0);
    test("(???1)!", "", PARSER_ERR_MP);
    test("???(1!)", "???1", 0);

    /* Powers */
    test("2??", "4", 0);
    test("2??", "8", 0);
    test("2?????", "1024", 0);
    test("(1+2)??", "9", 0);
    test("(x)??", "4", 0);
    test("|1???3|??", "4", 0);
    test("|x|??", "4", 0);
    test("0^0", "1", 0);
    test("0^0.5", "0", 0);
    test("2^0", "1", 0);
    test("2^1", "2", 0);
    test("2^2", "4", 0);
    test("2?????", "0.5", 0);
    test("2???", "", PARSER_ERR_MP);
    test("2^???1", "0.5", 0);
    test("2^(???1)", "0.5", 0);
    test("x?????", "0.5", 0);
    test("???10^2", "???100", 0);
    test("(???10)^2", "100", 0);
    test("???(10^2)", "???100", 0);
    test("2^100", "1267650600228229401496703205376", 0);
    test("4^3^2", "262144", 0);
    test("4^(3^2)", "262144", 0);
    test("(4^3)^2", "4096", 0);
    test("???4", "2", 0);
    test("???4???2", "0", 0);
    test("???8", "2", 0);
    test("???16", "2", 0);
    test("??????8", "2", 0);
    test("?????????1024", "2", 0);
    test("???(2+2)", "2", 0);
    test("2???4", "4", 0);
    test("2?????4", "4", 0);
    test("Sqrt(4)", "2", 0);
    test("Sqrt(2)", "1.414213562", 0);
    test("4^0.5", "2", 0);
    test("2^0.5", "1.414213562", 0);
    test("?????????8", "???2", 0);
    test("(???8)^(1??3)", "???2", 0);

    test("0 mod 7", "0", 0);
    test("6 mod 7", "6", 0);
    test("7 mod 7", "0", 0);
    test("8 mod 7", "1", 0);
    test("???1 mod 7", "6", 0);

    test("sgn 0", "0", 0);
    test("sgn 3", "1", 0);
    test("sgn ???3", "???1", 0);
    test("???3???", "3", 0);
    test("???3???", "3", 0);
    test("[3]", "3", 0);
    test("??????3???", "???3", 0);
    test("??????3???", "???3", 0);
    test("[???3]", "???3", 0);
    test("???3.2???", "3", 0);
    test("???3.2???", "4", 0);
    test("[3.2]", "3", 0);
    test("??????3.2???", "???4", 0);
    test("??????3.2???", "???3", 0);
    test("[???3.2]", "???3", 0);
    test("???3.5???", "3", 0);
    test("???3.5???", "4", 0);
    test("[3.5]", "4", 0);
    test("??????3.5???", "???4", 0);
    test("??????3.5???", "???3", 0);
    test("[???3.5]", "???4", 0);
    test("???3.7???", "3", 0);
    test("???3.7???", "4", 0);
    test("[3.7]", "4", 0);
    test("??????3.7???", "???4", 0);
    test("??????3.7???", "???3", 0);
    test("[???3.7]", "???4", 0);
    test("{3.2}", "0.2", 0);
    test("{???3.2}", "0.8", 0);

    test("|1|", "1", 0);
    test("|???1|", "1", 0);
    test("|3???5|", "2", 0);
    test("|x|", "2", 0);
    test("abs 1", "1", 0);
    test("abs (???1)", "1", 0);

    test("log 0", "", PARSER_ERR_MP);
    test("log 1", "0", 0);
    test("log 2", "0.301029996", 0);
    test("log 10", "1", 0);
    test("log?????? 10", "1", 0);
    test("log??? 2", "1", 0);
    test("2 log 2", "0.602059991", 0);

    test("ln 0", "", PARSER_ERR_MP);
    test("ln 1", "0", 0);
    test("ln 2", "0.693147181", 0);
    test("ln e", "1", 0);
    test("2 ln 2", "1.386294361", 0);

    options.angle_units = MP_DEGREES;
    test("sin 0", "0", 0);
    test("sin 45 ??? 1?????2", "0", 0);
    test("sin 20 + sin(???20)", "0", 0);
    test("sin 90", "1", 0);
    test("sin 180", "0", 0);
    test("2 sin 90", "2", 0);
    test("sin??45", "0.5", 0);

    test("cos 0", "1", 0);
    test("cos 45 ??? 1?????2", "0", 0);
    test("cos 20 ??? cos (???20)", "0", 0);
    test("cos 90", "0", 0);
    test("cos 180", "???1", 0);
    test("2 cos 0", "2", 0);
    test("cos??45", "0.5", 0);

    test("tan 0", "0", 0);
    test("tan 10 ??? sin 10??cos 10", "0", 0);
    test("tan 90", "", PARSER_ERR_MP);
    test("tan 10", "0.176326981", 0);
    test("tan??10", "0.031091204", 0);

    test("cos????? 0", "90", 0);
    test("cos????? 1", "0", 0);
    test("cos????? (???1)", "180", 0);
    test("cos????? (1?????2)", "45", 0);
    test("acos 0", "90", 0);
    test("acos 1", "0", 0);

    test("sin????? 0", "0", 0);
    test("sin????? 1", "90", 0);
    test("sin????? (???1)", "???90", 0);
    test("sin????? (1?????2)", "45", 0);
    test("asin 0", "0", 0);
    test("asin 1", "90", 0);

    test("cosh 0", "1", 0);
    test("cosh 10 ??? (e^10 + e^???10)??2", "0", 0);

    test("sinh 0", "0", 0);
    test("sinh 10 ??? (e^10 ??? e^???10)??2", "0", 0);
    test("sinh (???10) + sinh 10", "0", 0);

    test("cosh?? (???5) ??? sinh?? (???5)", "1", 0);
    test("tanh 0", "0", 0);
    test("tanh 10 ??? sinh 10 ?? cosh 10", "0", 0);

    test("atanh 0", "0", 0);
    test("atanh (1??10) ??? 0.5 ln(11??9)", "0", 0);

    options.angle_units = MP_DEGREES;
    test("sin 90", "1", 0);

    options.angle_units = MP_RADIANS;
    test("sin (????2)", "1", 0); // FIXME: Shouldn't need brackets

    options.angle_units = MP_GRADIANS;
    test("sin 100", "1", 0);

    /* Complex numbers */
    options.angle_units = MP_DEGREES;
    test("i", "i", 0);
    test("???i", "???i", 0);
    test("2i", "2i", 0);
    test("1+i", "1+i", 0);
    test("i+1", "1+i", 0);
    test("1???i", "1???i", 0);
    test("i???1", "???1+i", 0);
    test("i??i", "???1", 0);
    test("i??i", "1", 0);
    test("1??i", "???i", 0);
    test("|i|", "1", 0);
    test("|3+4i|", "5", 0);
    test("arg 0", "", PARSER_ERR_MP);
    test("arg 1", "0", 0);
    test("arg (1+i)", "45", 0);
    test("arg i", "90", 0);
    test("arg (???1+i)", "135", 0);
    test("arg ???1", "180", 0);
    test("arg (1+???i)", "???45", 0);
    test("arg ???i", "???90", 0);
    test("arg (???1???i)", "???135", 0);
    test("i?????", "???i", 0);
    test("??????1", "i", 0);
    test("(???1)^0.5", "i", 0);
    test("??????4", "2i", 0);
    test("e^i??", "???1", 0);
    test("ln (e^i??)", "3.141592654i", 0);
    test("log (???10) ??? (1 + ??i??ln(10))", "0", 0);
    test("ln (???e) ??? (1 + ??i)", "0", 0);
    test("sin(i????4) ??? i??sinh(????4)", "0", 0);
    test("cos(i????4) ??? cosh(????4)", "0", 0);

    /* Boolean */
    test("0 and 0", "0", 0);
    test("1 and 0", "0", 0);
    test("0 and 1", "0", 0);
    test("1 and 1", "1", 0);
    test("3 and 5", "1", 0);

    test("0 or 0", "0", 0);
    test("1 or 0", "1", 0);
    test("0 or 1", "1", 0);
    test("1 or 1", "1", 0);
    test("3 or 5", "7", 0);

    test("0 xor 0", "0", 0);
    test("1 xor 0", "1", 0);
    test("0 xor 1", "1", 0);
    test("1 xor 1", "0", 0);
    test("3 xor 5", "6", 0);

    options.base = 16;
    test("ones 1", "FFFFFFFE", 0);
    test("ones 7FFFFFFF", "80000000", 0);
    test("twos 1", "FFFFFFFF", 0);
    test("twos 7FFFFFFF", "80000001", 0);
    test("~7A??????", "FFFFFF85", 0);

    options.base = 2;
    options.wordlen = 4;
    test("1100???1010", "1000", 0);
    test("1100???1010", "1110", 0);
    test("1100???1010", "110", 0);
    test("1100???1010", "110", 0);
    //test("1100???1010", "0111", 0);
    //test("1100???1010", "0001", 0);
    //options.wordlen = 2;
    //test("??01???", "10???", 0);
    //test("????10???", "10???", 0);
}


int
main (int argc, char **argv)
{
    setlocale(LC_ALL, "C");

    test_conversions();
    test_equations();
    if (fails == 0)
        printf("Passed all %i tests\n", passes);

    return fails;
}
