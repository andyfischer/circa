# Copyright 2008 Andrew Fischer

header = "function list-apply(Function, List) -> List"
pure = True
evaluate = """
    as_function(caller->inputs[0]);
    List& list = as_list(caller->inputs[1]);

    as_list(caller).clear();

    for (int i=0; i < list.count(); i++) {
        Term* result = apply_function(*caller->owningBranch, caller->inputs[0], ReferenceList(list.get(i)));

        evaluate_term(result);

        as_list(caller).append(result);
    }
"""
