// Copyright 2008 Paul Hodge

#include "circa.h"


extern "C" {

void initialize()
{
    circa::initialize();
}

circa::Branch* new_branch()
{
    return new circa::Branch();
}

void evaluate_file(circa::Branch* branch, const char* filename)
{
    circa::evaluate_file(*branch, filename);
}

const char* print_raw_term(circa::Term* term)
{
    static std::string result;
    std::stringstream sstream;
    circa::print_raw_term(term, sstream);
    result = sstream.str();
    return result.c_str();
}

const char* print_raw_branch(circa::Branch* branch)
{
    static std::string result;
    std::stringstream sstream;
    circa::print_raw_branch(*branch, sstream);
    result = sstream.str();
    return result.c_str();
}

int term_num_inputs(circa::Term* term)
{
    return term->inputs.count();
}

circa::Term* term_get_input(circa::Term* term, int i)
{
    return term->inputs[i];
}

circa::Term* find_named(circa::Branch* branch, const char* name)
{
    return circa::find_named(branch, name);
}

}
