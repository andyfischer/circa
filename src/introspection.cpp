// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "function.h"
#include "introspection.h"
#include "runtime.h"
#include "term.h"
#include "type.h"

namespace circa {

std::string get_short_local_name(Term* term)
{
    if (term == NULL)
        return "NULL";
    if (term->name != "")
        return term->name;

    std::stringstream stream;
    stream << "#" << term->globalID;
    return stream.str();
}

void print_term_extended(Term* term, std::ostream &output)
{
    if (term == NULL) {
        output << "<NULL>";
        return;
    }

    std::string name = term->name;
    std::string funcName = get_short_local_name(term->function);
    std::string typeName = term->type->name;

    output << "#" << term->globalID;

    if (name != "")
        output << " " << name << "";

    output << ": ";

    output << funcName << "(";

    bool first_input = true;
    for (unsigned int input_index=0; input_index < term->inputs.count(); input_index++) {
        Term* input = term->inputs[input_index];
        if (!first_input) output << ", ";
        output << get_short_local_name(input);
        first_input = false;
    }

    output << ")";

    if (term->type != VOID_TYPE)
        output << " -> " << typeName;

    if (term->hasError())
        output << " *" << term->getErrorMessage() << "*";

    output << std::endl;
}

void print_branch_extended(Branch& branch, std::ostream &output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];

        print_term_extended(term, output);
    }
}

void print_terms(ReferenceList const& list, std::ostream &output)
{
    for (unsigned int i=0; i < list.count(); i++) {
        print_term_extended(list[i], output);
    }
}

ReferenceList list_all_pointers(Term* term)
{
    ReferenceList result;

    struct AppendPointersToList : PointerVisitor
    {
        ReferenceList& list;
        AppendPointersToList(ReferenceList &_list) : list(_list) {}

        virtual void visitPointer(Term* term) {
            if (term != NULL)
                list.appendUnique(term);
        }
    };

    AppendPointersToList visitor(result);
    visit_pointers(term, visitor);

    return result;
}

bool is_using(Term* user, Term* usee)
{
    assert_good_pointer(user);
    assert_good_pointer(usee);
    if (user->inputs.contains(usee))
        return true;
    if (user->function == usee)
        return true;

    return false;
}

bool function_allows_term_reuse(Function &function)
{
    if ((function.stateType != VOID_TYPE) && (function.stateType != NULL))
        return false;

    if (!function.pureFunction)
        return false;

    return true;
}

bool is_equivalent(Term* target, Term* function, ReferenceList const& inputs)
{
    if (target->function != function)
        return false;

    if (!function_allows_term_reuse(as_function(function)))
        return false;

    // Check inputs
    unsigned int numInputs = target->inputs.count();

    if (numInputs != inputs.count())
        return false;

    for (unsigned int i=0; i < numInputs; i++) {
        if (target->inputs[i] != inputs[i]) {
            return false;
        }
    }

    return true;
}

Term* find_equivalent(Branch &branch, Term* function, ReferenceList const& inputs)
{
    // This step is inefficient because we check every term in the branch.
    // Will improve in the future.
    for (int i=0; i < branch.numTerms(); i++) {
        Term *term = branch[i];
        if (is_equivalent(term, function, inputs))
            return term;
    }
    return NULL;
}

Term* find_equivalent(Term* function, ReferenceList const& inputs)
{
    if (!function_allows_term_reuse(as_function(function))) {
        return false;
    }

    Term* term = NULL;

    if (function->owningBranch != NULL)
        term = find_equivalent(*function->owningBranch, function, inputs);
    if (term != NULL)
        return term;

    // This step is inefficient because we check the owning branch of
    // every input, which may check some branches multiple times.
    for (unsigned int i=0; i < inputs.count(); i++) {
        if (inputs[i] == NULL)
            continue;

        Branch* branch = inputs[i]->owningBranch;
        if (branch != NULL)
            term = find_equivalent(*branch, function, inputs);
        if (term != NULL)
            return term;
    }

    return NULL;
}

void print_runtime_errors(Branch& branch, std::ostream& output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term *term = branch[i];

        if (term == NULL)
            continue;

        if (term->hasError()) {
            output << "error on " << term->name << ": " << term->getErrorMessage() << std::endl;
        }
    }
}

void check_for_compile_error(Term* term, std::string& errorMessage)
{
    if (term->function == UNKNOWN_FUNCTION) {
        errorMessage = std::string("Unknown function: " + as_string(term->state));
    }
}

bool has_compile_errors(Branch& branch)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        std::string message;
        check_for_compile_error(term, message);
        if (message != "")
            return true;
    }
    return false;
}

std::vector<std::string> get_compile_errors(Branch& branch)
{
    std::vector<std::string> results;

    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        std::string message;

        check_for_compile_error(term, message);

        if (message != "")
            results.push_back(message);
    }

    return results;
}

void print_compile_errors(Branch& branch, std::ostream& output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        std::string message;

        check_for_compile_error(term, message);

        if (message != "")
            output << message << std::endl;
    }
}

std::string get_source_of_input(Term* term, int inputIndex)
{
    switch (term->syntaxHints.getInputSyntax(inputIndex).style) {
        case TermSyntaxHints::InputSyntax::BY_SOURCE:
        {
            return get_term_source(term->input(inputIndex));
        }
        break;

        case TermSyntaxHints::InputSyntax::BY_NAME:
        {
            return term->input(inputIndex)->name;
        }
        break;

        case TermSyntaxHints::InputSyntax::UNKNOWN_STYLE:
        default:
        {
            return "(!unknown)";
        }
        break;
    }
}

bool should_print_term_source_line(Term* term)
{
    if (term->syntaxHints.occursInsideAnExpression)
        return false;

    return true;
}

std::string get_term_source(Term* term)
{
    std::stringstream result;

    // add possible name binding
    if (term->name != "") {
        result << term->name << " = ";
    }

    // add the declaration syntax
    switch (term->syntaxHints.declarationStyle) {
        case TermSyntaxHints::FUNCTION_CALL:
        {

            result << term->function->name << "(";

            for (int i=0; i < term->numInputs(); i++) {
                if (i > 0) result << ", ";

                result << get_source_of_input(term, i);
            }

            result << ")";
        }
        break;

        case TermSyntaxHints::LITERAL_VALUE:
        {
            result << to_source_string(term);
        }
        break;


        case TermSyntaxHints::UNKNOWN_DECLARATION_STYLE:
        {
            result << "(!error, unknown declaration style)";
        }
        break;

        case TermSyntaxHints::DOT_CONCATENATION:
        {
            result << get_source_of_input(term, 0);
            result << ".";
            result << term->function->name; /*<< "(";
            for (int i=1; i < term->numInputs(); i++) {
                if (i > 1) result << ", ";

                result << get_source_of_input(term, i);
            }
            result << ")";*/
        }
    }

    return result.str();
}

} // namespace circa
