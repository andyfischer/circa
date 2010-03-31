// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace int_t {
    bool equals(TaggedValue* a, TaggedValue* b)
    {
        if (is_float(b))
            return float_t::equals(a,b);
        if (!is_int(b))
            return false;
        return as_int(a) == as_int(b);
    }
    std::string to_string(TaggedValue* value)
    {
        std::stringstream strm;
        strm << as_int(value);
        return strm.str();
    }
    void format_source(StyledSource* source, Term* term)
    {
        std::stringstream strm;
        if (term->stringPropOptional("syntax:integerFormat", "dec") == "hex")
            strm << "0x" << std::hex;

        strm << as_int(term);
        append_phrase(source, strm.str(), term, token::INTEGER);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "int";
        type->equals = equals;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}

namespace float_t {
    void initialize(Type* type, TaggedValue* value)
    {
        set_float(value, 0);
    }

    void cast(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        set_float(dest, to_float(source));
    }

    bool cast_possible(Type* source, TaggedValue* value)
    {
        return is_int(value) || is_float(value);
    }

    bool equals(TaggedValue* a, TaggedValue* b)
    {
        return to_float(a) == to_float(b);
    }
    std::string to_string(TaggedValue* value)
    {
        std::stringstream out;
        out << as_float(value);
        return out.str();
    }

    std::string to_source_string(Term* term)
    {
        // Correctly formatting floats is a tricky problem.

        // First, check if we know how the user formatted this number. If this value
        // still has the exact same value, then use the original formatting.
        if (term->hasProperty("float:original-format")) {
            std::string const& originalFormat = term->stringProp("float:original-format");
            float actual = as_float(term);
            float original = (float) atof(originalFormat.c_str());
            if (actual == original) {
                return originalFormat;
            }
        }

        // Otherwise, format the current value with naive formatting. This could be
        // improved; we could try harder to recreate some of the original formatting.
        std::stringstream strm;
        strm << as_float(term);

        if (term->floatPropOptional("mutability", 0.0) > 0.5)
            strm << "?";

        std::string result = strm.str();

        // Check this string and make sure there is a decimal point. If not, append one.
        bool decimalFound = false;
        for (unsigned i=0; i < result.length(); i++)
            if (result[i] == '.')
                decimalFound = true;

        if (!decimalFound)
            return result + ".0";
        else
            return result;
    }
    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, float_t::to_source_string(term).c_str(), term, token::FLOAT_TOKEN);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "number";
        type->cast = cast;
        type->castPossible = cast_possible;
        type->equals = equals;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}

namespace bool_t {
    std::string to_string(TaggedValue* value)
    {
        if (as_bool(value))
            return "true";
        else
            return "false";
    }
    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, bool_t::to_string(term), term, token::BOOL);
    }
    void setup_type(Type* type)
    {
        type->name = "bool";
        type->toString = to_string;
        type->formatSource = format_source;
    }
}

namespace old_list_t {

    std::string to_string(Term* caller)
    {
        std::stringstream out;
        out << "[";
        Branch& branch = as_branch(caller);
        for (int i=0; i < branch.length(); i++) {
            if (i > 0) out << ",";
            out << branch[i]->toString();
        }
        out << "]";
        return out.str();
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "[", term, token::LBRACKET);
        Branch& branch = as_branch(term);
        for (int i=0; i < branch.length(); i++) {
            if (i > 0)
                append_phrase(source, ",", term, token::COMMA);
            format_term_source(source, branch[i]);
        }
        append_phrase(source, "]", term, token::RBRACKET);
    }

    void append(Branch& branch, Term* value)
    {
        //duplicate_value(branch, value);
        create_duplicate(branch, value);
    }

    void append(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        append(branch, value);
    }

    void count(EvalContext*, Term* caller)
    {
        set_int(caller, as_branch(caller->input(0)).length());
    }

    void setup(Type* type)
    {
        type->formatSource = formatSource;
    }

} // namespace old_list_t

namespace set_t {
    bool contains(Branch& branch, Term* value)
    {
        for (int i=0; i < branch.length(); i++)
            if (equals(value, branch[i]))
                return true;
        return false;
    }

    void add(Branch& branch, Term* value)
    {
        if (contains(branch, value))
            return;

        create_duplicate(branch, value);
    }

    void hosted_add(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
        Branch& contents = as_branch(caller);
        Term* value = caller->input(1);
        add(contents, value);
    }

    void contains(EvalContext*, Term* caller)
    {
        Branch& contents = as_branch(caller->input(0));
        Term* target = caller->input(1);
        set_bool(caller, contains(contents, target));
    }

    void remove(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
        Branch& contents = as_branch(caller);
        Term* value = caller->input(1);

        for (int index=0; index < contents.length(); index++) {
            if (equals(value, contents[index])) {
                contents.remove(index);
                return;
            }
        }
    }

    std::string to_string(TaggedValue* caller)
    {
        Branch &set = as_branch(caller);
        std::stringstream output;
        output << "{";
        for (int i=0; i < set.length(); i++) {
            if (i > 0) output << ", ";
            output << circa::to_string(set[i]);
        }
        output << "}";

        return output.str();
    }

    void setup_type(Type* type) {
        type->toString = set_t::to_string;

        Term* set_add = import_member_function(type, set_t::hosted_add, "add(Set, any) -> Set");
        function_set_use_input_as_output(set_add, 0, true);
        Term* set_remove = import_member_function(type, set_t::remove, "remove(Set, any) -> Set");
        function_set_use_input_as_output(set_remove, 0, true);
        import_member_function(type, set_t::contains, "contains(Set, any) -> bool");
    }

} // namespace set_t

namespace map_t {
    int find_key_index(Branch& contents, Term* key)
    {
        Branch& keys = contents[0]->asBranch();

        for (int i=0; i < keys.length(); i++)
            if (equals(keys[i], key))
                return i;
        return -1;
    }

    void insert(Branch& contents, Term* key, Term* value)
    {
        Branch& keys = contents[0]->asBranch();
        Branch& values = contents[1]->asBranch();

        int index = find_key_index(contents, key);

        if (index == -1) {
            create_duplicate(keys, key);
            create_duplicate(values, value);
        } else {
            copy(values[index], value);
        }
    }

    void remove(Branch& contents, Term* key)
    {
        Branch& keys = contents[0]->asBranch();
        Branch& values = contents[1]->asBranch();

        int index = find_key_index(contents, key);

        if (index != -1) {
            keys.set(index, NULL);
            keys.removeNulls();
            values.set(index, NULL);
            values.removeNulls();
        }
    }

    Term* get(Branch& contents, Term* key)
    {
        Branch& values = contents[1]->asBranch();
        int index = find_key_index(contents, key);

        if (index == -1)
            return NULL;
        else
            return values[index];
    }

    void contains(EvalContext*, Term* caller)
    {
        bool result = find_key_index(caller->input(0)->asBranch(), caller->input(1)) != -1;
        set_bool(caller, result);
    }

    void insert(EvalContext*, Term *caller)
    {
        copy(caller->input(0), caller);
        insert(caller->asBranch(), caller->input(1), caller->input(2));
    }

    void remove(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
        remove(caller->asBranch(), caller->input(1));
    }

    void get(EvalContext* cxt, Term* caller)
    {
        Term* key = caller->input(1);
        Term* value = get(caller->input(0)->asBranch(), key);
        if (value == NULL) {
            error_occurred(cxt, caller, "Key not found: " + to_string(key));
            return;
        }

        copy(value, caller);
    }

    std::string to_string(TaggedValue* value)
    {
        std::stringstream out;
        out << "{";

        Branch& contents = as_branch(value);
        Branch& keys = contents[0]->asBranch();
        Branch& values = contents[1]->asBranch();

        for (int i=0; i < keys.length(); i++) {
            if (i != 0)
                out << ", ";
            out << keys[i]->toString();
            out << ": ";
            out << values[i]->toString();
        }

        out << "}";
        return out.str();
    }

    void setup_type(Type* type)
    {
        type->toString = map_t::to_string;
        Term* map_add = import_member_function(type, map_t::insert, "add(Map, any, any) -> Map");
        function_set_use_input_as_output(map_add, 0, true);
        import_member_function(type, map_t::contains, "contains(Map, any) -> bool");
        Term* map_remove = import_member_function(type, map_t::remove, "remove(Map, any) -> Map");
        function_set_use_input_as_output(map_remove, 0, true);
        import_member_function(type, map_t::get, "get(Map, any) -> any");

    }
} // namespace map_t

namespace dict_t {
    std::string to_string(Branch& branch)
    {
        std::stringstream out;
        out << "{";
        for (int i=0; i < branch.length(); i++) {
            Term* term = branch[i];
            std::string name = term->name;
            if (name == "")
                name = "<anon>";

            if (i != 0)
                out << ", ";

            out << name << ": " << to_string(term);
        }
        out << "}";
        return out.str();
    }
} // namespace dict_t

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
        Branch& value = as_branch(term);

        bool valueHasAlpha = value[3]->asFloat() < 1.0;

        int specifiedDigits = term->intPropOptional("syntax:colorFormat", 6);

        int digitsPerChannel = (specifiedDigits == 6 || specifiedDigits == 8) ? 2 : 1;
        bool specifyAlpha = valueHasAlpha || (specifiedDigits == 4 || specifiedDigits == 8);

        std::stringstream out;

        out << "#";

        for (int c=0; c < 4; c++) {
            if (c == 3 && !specifyAlpha)
                break;

            double channel = std::min((double) value[c]->asFloat(), 1.0);

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
        append_phrase(source, color_t::to_string(term), term, token::COLOR);
    }

    void setup_type(Type* type)
    {
        type->formatSource = format_source;
    }

} // namespace color_t

namespace string_t {

    void initialize(Type* type, TaggedValue* value)
    {
        // temp:
        STRING_T = &as_type(STRING_TYPE);

        set_pointer(value, STRING_T, new std::string());
    }
    void release(TaggedValue* value)
    {
        delete ((std::string*) get_pointer(value));
        set_pointer(value, NULL);
    }

    void copy(TaggedValue* source, TaggedValue* dest)
    {
        *((std::string*) get_pointer(dest, STRING_T)) = as_string(source);
    }

    bool equals(TaggedValue* lhs, TaggedValue* rhs)
    {
        if (!is_string(rhs)) return false;
        return as_string(lhs) == as_string(rhs);
    }

    std::string to_string(TaggedValue* value)
    {
        std::stringstream result;
        result << "'" << as_string(value) << "'";
        return result.str();
    }

    void format_source(StyledSource* source, Term* term)
    {
        std::string quoteType = term->stringPropOptional("syntax:quoteType", "'");
        std::string result;
        if (quoteType == "<")
            result = "<<<" + as_string(term) + ">>>";
        else
            result = quoteType + as_string(term) + quoteType;

        append_phrase(source, result, term, token::STRING);
    }

    void length(EvalContext*, Term* term)
    {
        set_int(term, int(term->input(0)->asString().length()));
    }

    void substr(EvalContext*, Term* term)
    {
        set_str(term, as_string(term->input(0)).substr(int_input(term, 1), int_input(term, 2)));
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        STRING_T->name = "string";
        STRING_T->initialize = initialize;
        STRING_T->release = release;
        STRING_T->copy = copy;
        STRING_T->equals = equals;
        STRING_T->toString = to_string;
        STRING_T->formatSource = format_source;
    }

    void postponed_setup_type(Type* type)
    {
        import_member_function(type, length, "length(string) -> int");
        import_member_function(type, substr, "substr(string,int,int) -> string");
    }
}

namespace ref_t {
    std::string to_string(TaggedValue* term)
    {
        Term* t = as_ref(term);
        if (t == NULL)
            return "NULL";
        else
            return format_global_id(t);
    }
    void initialize(Type* type, TaggedValue* value)
    {
        // Temp:
        REF_T = &as_type(REF_TYPE);
        set_pointer(value, type, new Ref());
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        *((Ref*) get_pointer(dest, REF_T)) = as_ref(source);
    }
    bool equals(TaggedValue* lhs, TaggedValue* rhs)
    {
        return as_ref(lhs) == as_ref(rhs);
    }
    void get_name(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        set_str(caller, t->name);
    }
    void hosted_to_string(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        set_str(caller, circa::to_string(t));
    }
    void hosted_to_source_string(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        set_str(caller, circa::get_term_source_text(t));
    }
    void get_function(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        as_ref(caller) = t->function;
    }
    void assign(EvalContext* cxt, Term* caller)
    {
        Term* target = caller->input(0)->asRef();
        if (target == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }

        Term* source = caller->input(1);

        if (!value_fits_type(source, target->type)) {
            error_occurred(cxt, caller, "Can't assign (probably type mismatch)");
            return;
        }

        cast(source, target);
    }

    int round(double a) {
        return int(a + 0.5);
    }

    void tweak(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }

        int steps = caller->input(1)->asInt();

        if (steps == 0)
            return;

        if (is_float(t)) {
            float step = get_step(t);

            // do the math like this so that rounding errors are not accumulated
            float new_value = (round(as_float(t) / step) + steps) * step;
            set_float(t, new_value);

        } else if (is_int(t))
            set_int(t, as_int(t) + steps);
        else
            error_occurred(cxt, caller, "Ref is not an int or number");
    }
    void asint(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        if (!is_int(t)) {
            error_occurred(cxt, caller, "Not an int");
            return;
        }
        set_int(caller, as_int(t));
    }
    void asfloat(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        
        set_float(caller, to_float(t));
    }
    void get_input(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        int index = caller->input(1)->asInt();
        if (index >= t->numInputs())
            as_ref(caller) = NULL;
        else
            as_ref(caller) = t->input(index);
    }
    void num_inputs(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        set_int(caller, t->numInputs());
    }
    void get_source_location(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        Branch& output = as_branch(caller);
        set_int(output[0], t->intPropOptional("colStart", 0));
        set_int(output[1], t->intPropOptional("lineStart", 0));
    }
    void setup_type(Type* type)
    {
        type->name = "ref";
        type->remapPointers = Ref::remap_pointers;
        type->toString = to_string;
        type->initialize = initialize;
        type->copy = copy;
        type->equals = equals;
    }
}

namespace any_t {
    std::string to_string(Term* term)
    {
        return "<any>";
    }
}

namespace void_t {
    std::string to_string(TaggedValue*)
    {
        return "<void>";
    }
}


namespace type_t {
    void initialize(Type* type, TaggedValue* value)
    {
        set_pointer(value, type, new Type());
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        set_pointer(dest, dest->value_type, get_pointer(source, dest->value_type));
    }
}

namespace point_t {

    void read(Term* term, float* x, float* y)
    {
        Branch& branch = as_branch(term);
        *x = to_float(branch[0]);
        *y = to_float(branch[1]);
    }
    void write(Term* term, float x, float y)
    {
        Branch& branch = as_branch(term);
        set_float(branch[0], x);
        set_float(branch[1], y);
    }
}

void initialize_primitive_types(Branch& kernel)
{
    STRING_TYPE = create_type(kernel, "string");
    set_pointer(STRING_TYPE, STRING_T);

    INT_TYPE = create_type(kernel, "int");
    set_pointer(INT_TYPE, INT_T);

    FLOAT_TYPE = create_type(kernel, "number");
    set_pointer(FLOAT_TYPE, FLOAT_T);

    BOOL_TYPE = create_type(kernel, "bool");
    set_pointer(BOOL_TYPE, BOOL_T);

    REF_TYPE = create_type(kernel, "Ref");
    set_pointer(REF_TYPE, REF_T);

    // ANY_TYPE was created in bootstrap_kernel

    VOID_TYPE = create_empty_type(kernel, "void");
    Type* voidType = &as_type(VOID_TYPE);
    voidType->toString = void_t::to_string;
}

void post_setup_primitive_types()
{
    string_t::postponed_setup_type(&as_type(STRING_TYPE));

    import_member_function(REF_TYPE, ref_t::get_name, "name(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::hosted_to_string, "to_string(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::hosted_to_source_string,
            "to_source_string(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::get_function, "function(Ref) -> Ref");
    import_member_function(REF_TYPE, ref_t::assign, "assign(Ref, any)");
    import_member_function(REF_TYPE, ref_t::tweak, "tweak(Ref, int steps)");
    import_member_function(REF_TYPE, ref_t::asint, "asint(Ref) -> int");
    import_member_function(REF_TYPE, ref_t::asfloat, "asfloat(Ref) -> number");
    import_member_function(REF_TYPE, ref_t::get_input, "input(Ref, int) -> Ref");
    import_member_function(REF_TYPE, ref_t::num_inputs, "num_inputs(Ref) -> int");
    import_member_function(REF_TYPE, ref_t::get_source_location,
            "source_location(Ref) -> Point_i");
}

void setup_builtin_types(Branch& kernel)
{
    Term* branch_append = 
        import_member_function(BRANCH_TYPE, old_list_t::append, "append(Branch, any) -> Branch");
    function_set_use_input_as_output(branch_append, 0, true);

    import_member_function(TYPE_TYPE, type_t::name_accessor, "name(Type) -> string");

    Term* set_type = create_compound_type(kernel, "Set");
    set_t::setup_type(&as_type(set_type));

    // LIST_TYPE was created in bootstrap_kernel
    Term* list_append =
        import_member_function(LIST_TYPE, old_list_t::append, "append(List, any) -> List");
    function_set_use_input_as_output(list_append, 0, true);
    import_member_function(LIST_TYPE, old_list_t::count, "count(List) -> int");

    Term* map_type = create_compound_type(kernel, "Map");
    map_t::setup_type(&as_type(map_type));

    type_t::enable_default_value(map_type);
    Branch& map_default_value = type_t::get_default_value(map_type)->asBranch();
    create_list(map_default_value);
    create_list(map_default_value);

    NAMESPACE_TYPE = create_compound_type(kernel, "Namespace");
    OVERLOADED_FUNCTION_TYPE = create_compound_type(kernel, "OverloadedFunction");
    CODE_TYPE = create_compound_type(kernel, "Code");

    Term* branchRefType = parse_type(kernel, "type BranchRef { Ref target }");
    branch_ref_t::setup_type(&as_type(branchRefType));

    Term* styledSourceType = parse_type(kernel, "type StyledSource;");
    styled_source_t::setup_type(&as_type(styledSourceType));
}

void parse_builtin_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    color_t::setup_type(&as_type(COLOR_TYPE));

}

} // namespace circa
