// Copyright 2009 Paul Hodge

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace migration_snippets {

void test_migration(std::string sourceCode, std::string destinationCode, std::string assertionsCode)
{
    Branch source;
    parser::compile(&source, parser::statement_list, sourceCode);

    if (has_static_errors(source)) {
        std::cout << "In code: " << source;
        print_static_errors_formatted(source, std::cout);
        declare_current_test_failed();
        return;
    }

    Branch destination;
    parser::compile(&destination, parser::statement_list, destinationCode);

    if (has_static_errors(destination)) {
        std::cout << "In code: " << destination;
        print_static_errors_formatted(destination, std::cout);
        declare_current_test_failed();
        return;
    }

    evaluate_branch(source);

    migrate_stateful_values(source, destination);

    Branch& assertions = create_branch(destination, "assertions");
    parser::compile(&assertions, parser::statement_list, assertionsCode);

    evaluate_branch(destination);

    int boolean_statements_found = 0;
    for (int i=0; i < assertions.length(); i++) {
        if (!is_statement(assertions[i]))
            continue;

        if (!is_bool(assertions[i]))
            continue;

        boolean_statements_found++;

        if (!as_bool(assertions[i])) {
            std::cout << "In " << get_current_test_name() << std::endl;
            std::cout << "assertion failed: " << get_term_source(assertions[i]) << std::endl;
            std::cout << "Source:" << std::endl;
            std::cout << branch_to_string_raw(source);
            std::cout << "Destination:" << std::endl;
            std::cout << branch_to_string_raw(destination);
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0) {
        std::cout << "In " << get_current_test_name() << std::endl;
        std::cout << "warn: no boolean statements found in: " << assertionsCode << std::endl;
    }
}

void migrate_simple()
{
    test_migration("state i = 5", "state i = 4", "i == 5");
}

void register_tests()
{
    REGISTER_TEST_CASE(migration_snippets::migrate_simple);
}

} // namespace migration_snippets
} // namespace circa
