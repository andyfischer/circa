// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace int_t {
    void initialize(Type* type, TaggedValue* value)
    {
        set_int(value, 0);
    }
    bool equals(TaggedValue* a, TaggedValue* b)
    {
        if (is_float(b))
            return float_t::equals(a,b);
        if (!is_int(b))
            return false;
        return as_int(a) == as_int(b);
    }
    std::string to_string(Term* term)
    {
        std::stringstream strm;
        if (term->stringPropOptional("syntax:integerFormat", "dec") == "hex")
            strm << "0x" << std::hex;

        strm << as_int(term);
        return strm.str();
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

    bool equals(TaggedValue* a, TaggedValue* b)
    {
        return to_float(a) == to_float(b);
    }

    std::string to_string(Term* term)
    {
        // Correctly formatting floats is a tricky problem.

        // First, check if we know how the user typed this number. If this value
        // still has the exact same value, then use the original formatting.
        if (term->hasProperty("float:original-format")) {
            std::string const& originalFormat = term->stringProp("float:original-format");
            float actual = as_float(term);
            float original = (float) atof(originalFormat.c_str());
            if (actual == original) {
                return originalFormat;
            }
        }

        // Otherwise, format the current value with naive formatting
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
}

namespace bool_t {
    std::string to_string(Term* term)
    {
        if (as_bool(term))
            return "true";
        else
            return "false";
    }
}

namespace list_t {

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

    void append(Branch& branch, Term* value)
    {
        //duplicate_value(branch, value);
        create_duplicate(branch, value);
    }

    void append(EvalContext*, Term* caller)
    {
        assign_overwriting_type(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        append(branch, value);
    }

    void count(EvalContext*, Term* caller)
    {
        set_int(caller, as_branch(caller->input(0)).length());
    }

} // namespace list_t

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
        assign_overwriting_type(caller->input(0), caller);
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
        assign_overwriting_type(caller->input(0), caller);
        Branch& contents = as_branch(caller);
        Term* value = caller->input(1);

        for (int index=0; index < contents.length(); index++) {
            if (equals(value, contents[index])) {
                contents.remove(index);
                return;
            }
        }
    }

    std::string to_string(Term* caller)
    {
        Branch &set = as_branch(caller);
        std::stringstream output;
        output << "{";
        for (int i=0; i < set.length(); i++) {
            if (i > 0) output << ", ";
            output << set[i]->toString();
        }
        output << "}";

        return output.str();
    }

    void setup(Branch& kernel) {
        Term* set_type = create_compound_type(kernel, "Set");
        as_type(set_type).toString = set_t::to_string;

        Term* set_add = import_member_function(set_type, set_t::hosted_add, "add(Set, any) -> Set");
        function_set_use_input_as_output(set_add, 0, true);
        Term* set_remove = import_member_function(set_type, set_t::remove, "remove(Set, any) -> Set");
        function_set_use_input_as_output(set_remove, 0, true);
        import_member_function(set_type, set_t::contains, "contains(Set, any) -> bool");
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
            assign_overwriting_type(values[index], value);
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
        assign_overwriting_type(caller->input(0), caller);
        insert(caller->asBranch(), caller->input(1), caller->input(2));
    }

    void remove(EvalContext*, Term* caller)
    {
        assign_overwriting_type(caller->input(0), caller);
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

        assign_overwriting_type(value, caller);
    }

    std::string to_string(Term* term)
    {
        std::stringstream out;
        out << "{";

        Branch& contents = as_branch(term);
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
} // namespace color_t

namespace branch_ref_t
{
    bool is_considered_config(Term* term)
    {
        if (term == NULL) return false;
        if (term->name == "") return false;
        if (!is_value(term)) return false;
        if (is_stateful(term)) return false;
        if (is_hidden(term)) return false;

        // ignore branch-based types
        if (is_branch(term)) return false;
        if (is_type(term)) return false;

        return true;
    }

    Branch& get_target_branch(Term* caller)
    {
        Branch& mirrorObject = caller->input(0)->asBranch();
        return mirrorObject[0]->asRef()->asBranch();
    }

    void get_configs(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        Branch& output = caller->asBranch();

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
    }
    void get_configs_nested(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        Branch& output = caller->asBranch();

        int write = 0;
        for (BranchIterator it(target_branch); !it.finished(); it.advance()) {
            Term* t = *it;

            if (is_branch(t)) {
                // check if we should explore this branch
                bool explore = (t->type == CODE_TYPE || is_function(t))
                        && !is_hidden(t) && t->name != "";

                if (!explore)
                    it.skipNextBranch();
                
                continue;
            }

            if (!is_considered_config(t))
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
    }
    void get_visible(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        Branch& output = caller->asBranch();

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (is_hidden(t) || t->function == COMMENT_FUNC)
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
    }

    void get_relative_name(EvalContext* cxt, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        Term* target = caller->input(1)->asRef();

        if (target == NULL) {
            error_occurred(cxt, caller, "term is NULL");
            return;
        }

        set_str(caller, get_relative_name(target_branch, target));
    }

    void get_length(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        set_int(caller, target_branch.length());
    }
    void get_index(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        int index = int_input(caller, 1);
        if (index >= target_branch.length())
            set_ref(caller, NULL);
        else
            set_ref(caller, target_branch[index]);
    }
    void append_code(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        Branch& input = as_branch(caller->input(1));

        if (input.length() == 0)
            return;

        int previousLast = target_branch.length();

        Branch temp_copy;
        duplicate_branch(input, temp_copy);
        lift_closure(temp_copy);

        duplicate_branch(temp_copy, target_branch);

        // Strip trailing whitespace after the formally-last and newly-last terms
        // so that the resulting source looks better.
        if (previousLast > 0)
            target_branch[previousLast-1]->removeProperty("syntax:lineEnding");
        target_branch[target_branch.length()-1]->removeProperty("syntax:lineEnding");
    }
    void print_raw(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);

        set_str(caller, get_branch_raw(target_branch));
    }
    void save(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        persist_branch_to_file(target_branch);
    }
    void to_source(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller);
        set_str(caller, get_branch_source(target_branch));
    }

} // namespace branch_ref_t

namespace string_t {

    void initialize(Type* type, TaggedValue* value)
    {
        // temp:
        STRING_T = &as_type(STRING_TYPE);

        set_pointer(value, STRING_T, new std::string());
    }
    void destroy(Type* type, TaggedValue* value)
    {
        delete ((std::string*) get_pointer(value));
        set_pointer(value, NULL);
    }

    void assign(TaggedValue* source, TaggedValue* dest)
    {
        *((std::string*) get_pointer(dest, STRING_T)) = as_string(source);
    }

    bool equals(TaggedValue* lhs, TaggedValue* rhs)
    {
        if (!is_string(rhs)) return false;
        return as_string(lhs) == as_string(rhs);
    }

    std::string to_string(Term* term)
    {
        std::string quoteType = term->stringPropOptional("syntax:quoteType", "'");
        if (quoteType == "<") return "<<<" + as_string(term) + ">>>";
        else return quoteType + as_string(term) + quoteType;
    }

    void length(EvalContext*, Term* term)
    {
        set_int(term, int(term->input(0)->asString().length()));
    }

    void substr(EvalContext*, Term* term)
    {
        set_str(term, as_string(term->input(0)).substr(int_input(term, 1), int_input(term, 2)));
    }
}

namespace ref_t {
    std::string to_string(Term* term)
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
    void assign(TaggedValue* source, TaggedValue* dest)
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
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        set_str(caller, t->name);
    }
    void hosted_to_string(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        set_str(caller, circa::to_string(t));
    }
    void get_function(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        as_ref(caller) = t->function;
    }
    void hosted_typeof(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        as_ref(caller) = t->type;
    }
    void assign(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }

        Term* source = caller->input(1);

        if (!is_assign_value_possible(source, t)) {
            error_occurred(cxt, caller, "Can't assign (probably type mismatch)");
            return;
        }

        assign_value(source, t);
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
}

namespace any_t {
    std::string to_string(Term* term)
    {
        return "<any>";
    }
}

namespace void_t {
    std::string to_string(Term*)
    {
        return "<void>";
    }
}

namespace type_t {
    void initialize(Type* type, TaggedValue* value)
    {
        set_pointer(value, type, new Type());
    }
    void assign(TaggedValue* source, TaggedValue* dest)
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
    Type* anyType = &as_type(ANY_TYPE);
    anyType->toString = any_t::to_string;

    VOID_TYPE = create_empty_type(kernel, "void");
    Type* voidType = &as_type(VOID_TYPE);
    voidType->toString = void_t::to_string;
}

void post_setup_primitive_types()
{
    import_member_function(STRING_TYPE, string_t::length, "length(string) -> int");
    import_member_function(STRING_TYPE, string_t::substr, "substr(string,int,int) -> string");

    import_member_function(REF_TYPE, ref_t::get_name, "name(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::hosted_to_string, "to_string(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::hosted_typeof, "typeof(Ref) -> Ref");
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
        import_member_function(BRANCH_TYPE, list_t::append, "append(Branch, any) -> Branch");
    function_set_use_input_as_output(branch_append, 0, true);

    import_member_function(TYPE_TYPE, type_t::name_accessor, "name(Type) -> string");

    set_t::setup(kernel);

    // LIST_TYPE was created in bootstrap_kernel
    Term* list_append =
        import_member_function(LIST_TYPE, list_t::append, "append(List, any) -> List");
    function_set_use_input_as_output(list_append, 0, true);
    import_member_function(LIST_TYPE, list_t::count, "count(List) -> int");

    Term* map_type = create_compound_type(kernel, "Map");
    as_type(map_type).toString = map_t::to_string;
    Term* map_add = import_member_function(map_type, map_t::insert, "add(Map, any, any) -> Map");
    function_set_use_input_as_output(map_add, 0, true);
    import_member_function(map_type, map_t::contains, "contains(Map, any) -> bool");
    Term* map_remove = import_member_function(map_type, map_t::remove, "remove(Map, any) -> Map");
    function_set_use_input_as_output(map_remove, 0, true);
    import_member_function(map_type, map_t::get, "get(Map, any) -> any");

    type_t::enable_default_value(map_type);
    Branch& map_default_value = type_t::get_default_value(map_type)->asBranch();
    create_list(map_default_value);
    create_list(map_default_value);

    NAMESPACE_TYPE = create_compound_type(kernel, "Namespace");
    OVERLOADED_FUNCTION_TYPE = create_compound_type(kernel, "OverloadedFunction");
    CODE_TYPE = create_compound_type(kernel, "Code");

    BRANCH_REF_TYPE = parse_type(kernel, "type BranchRef { Ref target }");
}

void parse_builtin_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");
    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    as_type(COLOR_TYPE).toString = color_t::to_string;

    import_member_function(BRANCH_REF_TYPE, branch_ref_t::get_configs,
        "get_configs(BranchRef) -> List");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::get_configs_nested,
        "get_configs_nested(BranchRef) -> List");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::get_relative_name,
        "get_relative_name(BranchRef, Ref) -> string");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::get_visible,
        "get_visible(BranchRef) -> List");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::get_length,
        "length(BranchRef) -> int");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::get_index,
        "get_index(BranchRef, int) -> Ref");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::append_code,
        "append_code(BranchRef, Branch)");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::print_raw,
        "print_raw(BranchRef) -> string");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::save,
        "save(BranchRef)");
    import_member_function(BRANCH_REF_TYPE, branch_ref_t::to_source,
        "to_source(BranchRef) -> string");
}

} // namespace circa
