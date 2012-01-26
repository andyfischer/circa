// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "source_repro.h"
#include "token.h"

namespace circa {
namespace color_t {
    char number_to_hex_digit(int n) {
        if (n >= 0 && n <= 9)
            return '0' + n;

        if (n >= 10 && n <= 16)
            return 'a' + (n - 10);

        return 'f';
    }

    std::string to_string(Term* term)
    {
        TValue* value = term;

        bool valueHasAlpha = value->getIndex(3)->asFloat() < 1.0;

        int specifiedDigits = term->intPropOptional("syntax:colorFormat", 6);

        int digitsPerChannel = (specifiedDigits == 6 || specifiedDigits == 8) ? 2 : 1;
        bool specifyAlpha = valueHasAlpha || (specifiedDigits == 4 || specifiedDigits == 8);

        std::stringstream out;

        out << "#";

        for (int c=0; c < 4; c++) {
            if (c == 3 && !specifyAlpha)
                break;

            double channel = std::min((double) value->getIndex(c)->asFloat(), 1.0);

            if (digitsPerChannel == 1)
                out << number_to_hex_digit(int(channel * 15.0));
            else {
                int mod_255 = int(channel * 255.0);
                out << number_to_hex_digit(mod_255 / 0x10);
                out << number_to_hex_digit(mod_255 % 0x10);
            }
        }

        return out.str();
    }

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, color_t::to_string(term), term, TK_COLOR);
    }

    void setup_type(Type* type)
    {
        type->formatSource = format_source;
    }

} // namespace color_t

void get_color(TValue* value, float* r, float* g, float* b, float* a)
{
    *r = to_float(get_index(value, 0));
    *g = to_float(get_index(value, 1));
    *b = to_float(get_index(value, 2));
    *a = to_float(get_index(value, 3));
}

} // namespace circa
