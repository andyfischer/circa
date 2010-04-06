// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

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
        if (term->hasProperty("syntax:originalString")) {
            append_phrase(source, term->stringProp("syntax:originalString"),
                    term, token::STRING);
            return;
        }

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

namespace any_t {
    std::string to_string(TaggedValue*)
    {
        return "<any>";
    }
    bool matches_type(Type*, Term*)
    {
        return true;
    }
}

namespace void_t {
    std::string to_string(TaggedValue*)
    {
        return "<void>";
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
    ref_t::postponed_setup_type(&as_type(REF_TYPE));
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

    Term* indexableType = parse_type(kernel, "type Indexable;");
    indexable_t::setup_type(&as_type(indexableType));

    callable_t::setup_type(&as_type(parse_type(kernel, "type Callable;")));
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
