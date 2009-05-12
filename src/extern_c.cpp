// Copyright 2008 Andrew Fischer

// extern_c.cpp
//
// In this file, we provide various public functions in an extern "C" block, so that this library
// can be loaded as a C library. (for example, with Python's ctypes)

#include "circa.h"

using namespace circa;

extern "C" {

void initialize()
{
    circa::initialize();
}

Branch* new_branch()
{
    return new Branch();
}

void parse_file(Branch* branch, const char* filename)
{
    circa::parse_file(*branch, filename);
}

const char* term_to_raw_string(Term* term)
{
    static std::string result;
    result = circa::term_to_raw_string(term);
    return result.c_str();
}

const char* branch_to_string_raw(Branch* branch)
{
    static std::string result;
    result = circa::branch_to_string_raw(*branch);
    return result.c_str();
}

int term_num_inputs(Term* term) { return (int) term->inputs.count(); }
Term* term_get_input(Term* term, int i) { return term->inputs[i]; }

const char* term_get_name(Term* term)
{
    static std::string result;
    result = term->name;
    return result.c_str();
}
const char* to_string(Term* term)
{
    static std::string result;
    result = circa::to_string(term);
    return result.c_str();
}

Term* find_named(Branch* branch, const char* name) { return circa::find_named(branch, name); }
bool has_inner_branch(Term* term) { return circa::has_inner_branch(term); }
Branch* get_inner_branch(Term* term) { return circa::get_inner_branch(term); }
void reload_branch_from_file(Branch* branch) { circa::reload_branch_from_file(*branch); }
Term* find_term_by_id(Branch* branch, unsigned int i) { return find_term_by_id(*branch, i); }
void evaluate_term(Term* term) { circa::evaluate_term(term); }

}
