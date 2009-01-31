// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "parser.h"
#include "ref_list.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"
#include "wrappers.h"

namespace circa {


bool push_is_inside_expression(Branch& branch, bool value)
{
    bool previous;

    if (branch.containsName(get_name_for_attribute("comp-inside-expr"))) {
        previous = as_bool(branch[get_name_for_attribute("comp-inside-expr")]);
    } else {
        bool_value(branch, BOOL_TYPE, get_name_for_attribute("comp-inside-expr"));
        previous = false;
    }

    as_bool(branch[get_name_for_attribute("comp-inside-expr")]) = value;

    return previous;
}

void pop_is_inside_expression(Branch& branch, bool value)
{
    as_bool(branch[get_name_for_attribute("comp-inside-expr")]) = value;
}

bool is_inside_expression(Branch& branch)
{
    if (branch.containsName(get_name_for_attribute("comp-inside-expr")))
        return as_bool(branch[get_name_for_attribute("comp-inside-expr")]);
    else
        return false;
}

void push_pending_rebind(Branch& branch, std::string const& name)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.containsName(attrname))
        throw std::runtime_error("pending rebind already exists");

    string_value(branch, name, attrname);
}

std::string get_pending_rebind(Branch& branch)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.containsName(attrname))
        return as_string(branch[attrname]);
    else
        return "";
}

void remove_compilation_attrs(Branch& branch)
{
    branch.removeTerm(get_name_for_attribute("comp-inside-expr"));
    branch.removeTerm(get_name_for_attribute("comp-pending-rebind"));
}

Term* find_and_apply_function(Branch& branch,
        std::string const& functionName,
        ReferenceList inputs)
{
    Term* function = find_named(&branch, functionName);

    // If function is not found, produce an instance of unknown-function
    if (function == NULL) {
        Term* result = apply_function(&branch,
                                      UNKNOWN_FUNCTION,
                                      inputs);
        as_string(result->state) = functionName;
        return result;
    }

    return apply_function(&branch, function, inputs);
}

Term* create_comment(Branch& branch, std::string const& text)
{
    Term* result = apply_function(&branch, COMMENT_FUNC, ReferenceList());
    as_string(result->state->field(0)) = text;
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    result->syntaxHints.occursInsideAnExpression = false;
    return result;
}

Term* create_literal_string(Branch& branch, ast::LiteralString& ast)
{
    Term* term = string_value(branch, ast.text);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    return term;
}

Term* create_literal_float(Branch& branch, ast::LiteralFloat& ast)
{
    float value = atof(ast.text.c_str());
    Term* term = float_value(branch, value);
    float mutability = ast.hasQuestionMark ? 1.0 : 0.0;
    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    return term;
}

Term* create_literal_integer(Branch& branch, ast::LiteralInteger& ast)
{
    int value = strtol(ast.text.c_str(), NULL, 0);
    Term* term = int_value(branch, value);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    return term;
}

Term* create_dot_concatenated_call(Branch &branch,
                                   ast::Infix& ast)
{
    bool previousIsInsideExpression = push_is_inside_expression(branch, true);
    Term* leftTerm = ast.left->createTerm(branch);
    pop_is_inside_expression(branch, previousIsInsideExpression);

    // Figure out the function name. Right expression might be
    // an identifier or a function call
    std::string functionName;

    if (ast.right->typeName() == "Identifier")
        functionName = dynamic_cast<ast::Identifier*>(ast.right)->text;
    else if (ast.right->typeName() == "FunctionCall")
        functionName = dynamic_cast<ast::FunctionCall*>(ast.right)->functionName;
    else
        parser::syntax_error(ast.right->typeName() + " on right side of infix dot");

    Type &leftType = as_type(leftTerm->type);

    // Try to find the function. Check the type's member function first.
    Term* function = NULL;

    bool memberFunctionCall = false;

    if (leftType.memberFunctions.contains(functionName)) {
        function = leftType.memberFunctions[functionName];
        memberFunctionCall = true;

    } else {
        function = find_named(&branch,functionName);
    }

    if (function == NULL)
        parser::syntax_error(functionName + " function not found.");

    // Assemble input list
    ReferenceList inputs(leftTerm);

    if (ast.right->typeName() == "FunctionCall") {
        ast::FunctionCall* functionCall = dynamic_cast<ast::FunctionCall*>(ast.right);

        for (unsigned int i=0; i < functionCall->arguments.size(); i++) {
            ast::FunctionCall::Argument *arg = functionCall->arguments[i];
            bool previousIsInsideExpression = push_is_inside_expression(branch, true);
            Term *term = arg->expression->createTerm(branch);
            pop_is_inside_expression(branch, previousIsInsideExpression);
            inputs.append(term);
        }
    }

    Term* result = apply_function(&branch, function, inputs);

    if (memberFunctionCall
            && (ast.left->typeName() == "Identifier")
            && (ast.right->typeName() == "FunctionCall")) {

        // Rebind this identifier
        std::string id = dynamic_cast<ast::Identifier*>(ast.left)->text;

        branch.bindName(result, id);
    }

    TermSyntaxHints::InputSyntax leftInputSyntax;
    if (ast.left->typeName() == "Identifier")
        leftInputSyntax.style = TermSyntaxHints::InputSyntax::BY_NAME;
    else 
        leftInputSyntax.style = TermSyntaxHints::InputSyntax::BY_SOURCE;

    result->syntaxHints.inputSyntax.push_back(leftInputSyntax);
    result->syntaxHints.declarationStyle = TermSyntaxHints::DOT_CONCATENATION;
    result->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);

    return result;
}

Term* create_arrow_concatenated_call(Branch &branch, ast::Infix& ast)
{
    bool previousIsInsideExpression = push_is_inside_expression(branch, true);
    Term* leftTerm = ast.left->createTerm(branch);
    pop_is_inside_expression(branch, previousIsInsideExpression);

    ast::Identifier *rightIdent = dynamic_cast<ast::Identifier*>(ast.right);

    if (rightIdent == NULL) {
        parser::syntax_error("Right side of -> must be an identifier");
    }

    return find_and_apply_function(branch, rightIdent->text, ReferenceList(leftTerm));
}

Term* create_feedback_call(Branch &branch, ast::Infix& ast)
{
    bool previousIsInsideExpression = push_is_inside_expression(branch, true);
    Term* leftTerm = ast.left->createTerm(branch);
    Term* rightTerm = ast.right->createTerm(branch);
    pop_is_inside_expression(branch, previousIsInsideExpression);

    return apply_function(&branch, APPLY_FEEDBACK, ReferenceList(leftTerm, rightTerm));
}

TermSyntaxHints::InputSyntax get_input_syntax(ast::ASTNode* node)
{
    TermSyntaxHints::InputSyntax result;

    if (node->typeName() == "Identifier") {
        result.style = TermSyntaxHints::InputSyntax::BY_NAME;
        result.name = dynamic_cast<ast::Identifier*>(node)->text;
    } else {
        result.style = TermSyntaxHints::InputSyntax::BY_SOURCE;
    }

    return result;
}

Term* create_infix_call(Branch &branch, ast::Infix& ast)
{
    std::string functionName = getInfixFunctionName(ast.operatorStr);

    Term* function = find_named(&branch,functionName);

    if (function == NULL) {
        parser::syntax_error(std::string("couldn't find function: ") + functionName);
        return NULL; // unreachable
    }

    bool previousIsInsideExpression = push_is_inside_expression(branch, true);
    Term* leftTerm = ast.left->createTerm(branch);
    Term* rightTerm = ast.right->createTerm(branch);
    pop_is_inside_expression(branch, previousIsInsideExpression);
    
    Term* result = apply_function(&branch, function, ReferenceList(leftTerm, rightTerm));

    result->syntaxHints.setInputSyntax(0, get_input_syntax(ast.left));
    result->syntaxHints.setInputSyntax(1, get_input_syntax(ast.right));
    result->syntaxHints.declarationStyle = TermSyntaxHints::INFIX;
    result->syntaxHints.functionName = ast.operatorStr;
    result->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);

    return result;
}

bool is_literal(ast::ASTNode* node)
{
    return (node->typeName() == "LiteralInteger")
        || (node->typeName() == "LiteralFloat")
        || (node->typeName() == "LiteralString");
}

Term* create_function_call(Branch &branch, ast::FunctionCall& ast)
{
    ReferenceList inputs;

    for (unsigned int i=0; i < ast.arguments.size(); i++) {
        ast::FunctionCall::Argument* arg = ast.arguments[i];
        bool previousIsInsideExpression = push_is_inside_expression(branch, true);
        Term* term = arg->expression->createTerm(branch);
        pop_is_inside_expression(branch, previousIsInsideExpression);

        inputs.append(term);
    }

    Term* result = find_and_apply_function(branch, ast.functionName, inputs);
    assert(result != NULL);

    for (unsigned int i=0; i < ast.arguments.size(); i++) {

        result->syntaxHints.inputSyntax.push_back(
                get_input_syntax(ast.arguments[i]->expression));
    }

    result->syntaxHints.declarationStyle = TermSyntaxHints::FUNCTION_CALL;
    result->syntaxHints.functionName = ast.functionName;
    result->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);

    return result;
}

Term* create_stateful_value_declaration(Branch &branch,
        ast::StatefulValueDeclaration& ast)
{
    Term* initialValue = NULL;
    
    if (ast.initialValue != NULL)
        initialValue = ast.initialValue->createTerm(branch);

    Term* type = find_named(&branch, ast.type);

    Term* value = create_value(&branch, type);
    value->setIsStateful(true);

    if (initialValue != NULL)
        assign_value(initialValue, value);

    branch.bindName(value, ast.name);

    return value;
}

} // namespace circa
