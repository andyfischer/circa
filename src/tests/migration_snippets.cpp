// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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

    // Test specializing the destination type
    test_migration("state i = 5", "state any i", "i == 5");
    test_migration("state List i = [] \n i.append(1)", "state any i", "i == [1]");
}

void migrate_across_user_defined_types()
{
    // Type T is defined the same way
    test_migration("type T { int x } \n state T t = [1]",
        "type T { int x } \n state T t = [2]",
        "t.x == 1");

    // Type T has the same data types, but with a different field name
    test_migration("type T { int x } \n state T t = [1]",
        "type T { int y } \n state T t = [2]",
        "t.y == 1");
}

void migrate_misc()
{
    // This tests don't have a specific focus

    test_migration("type Point { number x, number y }\n"
                   "def get_ship_start_point() : Point\n return [50,50]\n end\n"
                   "type Ship { Point loc, Point momentum, number facing }\n"
                   "state Ship ship = [get_ship_start_point() [0 0] 0]\n"
                   "ship = [[5 5] [1 1] 1]",

                   "type Point { number x, number y }\n"
                   "def get_ship_start_point() : Point\n return [50,50]\n end\n"
                   "type Ship { Point loc, Point momentum, number facing }\n"
                   "state Ship ship = [get_ship_start_point() [0 0] 0]\n",

                   "ship.loc == [5.0 5.0], ship.momentum == [1.0 1.0], ship.facing == 1.0");
}

void register_tests()
{
    REGISTER_TEST_CASE(migration_snippets::migrate_simple);
    REGISTER_TEST_CASE(migration_snippets::migrate_across_user_defined_types);
    REGISTER_TEST_CASE(migration_snippets::migrate_misc);
}

} // namespace migration_snippets
} // namespace circa
