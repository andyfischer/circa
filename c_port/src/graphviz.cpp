
#include "circa.h"

void export_graphviz__evaluate(circa::Term* caller)
{
    circa::Subroutine *sub = circa::as_subroutine(caller->inputs[0]);
    std::string filename = circa::as_string(caller->inputs[1]);

}

void create_functions(circa::Branch* branch)
{
    circa::quick_create_function(branch, "export-graphviz",
            export_graphviz__evaluate,
            circa::TermList(circa::SUBROUTINE_TYPE, circa::STRING_TYPE),
            circa::VOID_TYPE);
}
