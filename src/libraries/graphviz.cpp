
#include "circa.h"

void export_graphviz__evaluate(circa::Term* caller)
{
    /*
    circa::Subroutine &sub = circa::as_subroutine(caller->inputs[0]);

    std::stringstream output;

    output << "digraph G {" << std::endl;

    for (int i=0; i < sub.branch.numTerms(); i++) {
        circa::Term* term = sub.branch.get(i);
        std::string termStr = term->toString();

        for (int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++) {
            std::string inputStr = term->inputs[inputIndex]->toString();
            output << inputStr << " -> " << termStr << ";\n";
        }
    }

    output << "}" << std::endl;

    circa::as_string(caller) = output.str();
    */
}

void graphviz_initialize(circa::Branch* branch)
{
    /*circa::quick_create_function(branch, "export-graphviz",
            export_graphviz__evaluate,
            circa::ReferenceList(circa::SUBROUTINE_TYPE),
            circa::STRING_TYPE);*/
}
