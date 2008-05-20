
#include "Errors.h"
#include "Function.h"
#include "Term.h"
#include "Type.h"

Term* Term::input(int index)
{
    return this->inputs[index];
}

Term* Term::get_type() const
{
    if (this->function == NULL) {
        printf("ERROR: In get_type, term %s has NULL function.\n",
            this->debug_name.c_str());
        return NULL;
    }

    return function::output_type(this->function);
}

string Term::to_string() const
{
    if (this == NULL) {
        printf("ERROR: Term::to_string called on null pointer.\n");
        return "";
    }

    if (this->get_type() == NULL) {
        printf("ERROR: %s has a null type.\n", this->debug_name.c_str());
        return "";
    }

    return type::call_to_string(this->get_type(), const_cast<Term*>(this));
}

string Term::debug_identifier() const
{
    string output = to_string();

    if (this->debug_name != "") {
        output += " (" + this->debug_name + ")";
    }
    return output;
}

void Term::evaluate()
{
    if (function::evaluate_func(this->function) == NULL) {
        INTERNAL_ERROR(string("Function ") + this->function->debug_identifier()
           + " has no evaluate function.");
    }
    function::evaluate_func(this->function)(this);
    this->needs_update = false;
}

