#!/usr/bin/env python

import os, string

TEMPLATE = string.Template("""
// Copyright 2008 Paul Hodge

// This file was generated using $source_file. You should probably not modify
// this file directly.

namespace ${name}_function {

    void evaluate(Term* caller)
    {
        $evaluate
    }

    $feedback_propogate_defn
    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "$header");
        as_function(main_func).pureFunction = $is_pure;

        $feedback_propogate_setup
    }
}
""")

FEEDBACK_PROPOGATE_DEFN_TEMPLATE = string.Template("""
    void feedback_propogate(Term* caller)
    {
        $feedback_propogate
    }
""")

FEEDBACK_PROPOGATE_SETUP_TEMPLATE = string.Template("""
        Term* fp_func = import_c_function(kernel, feedback_propogate,
                "function ${name}-feedback-propogate(any,any)");
        as_function(fp_func).stateType = BRANCH_TYPE;
        as_function(main_func).feedbackPropogateFunction = fp_func;
""")

def generate_function(functionName):

    root_path = os.path.join('..','src','builtin_functions')
    py_source_path = os.path.join(root_path, functionName + '.source.py')
    output_path = os.path.join(root_path, functionName + '.cpp')

    if not os.path.exists(py_source_path):
        raise Exception("couldn't find file: "+py_source_path)
    if not os.path.exists(output_path):
        raise Exception("couldn't find file: "+output_path)

    configs = {'name':functionName, 'source_file':py_source_path}

    execfile(py_source_path, configs)

    configs['is_pure'] = 'true' if configs['pure'] else 'false'

    if 'feedback_propogate' in configs:
        configs['feedback_propogate_defn'] = FEEDBACK_PROPOGATE_DEFN_TEMPLATE.substitute(configs)
        configs['feedback_propogate_setup'] = FEEDBACK_PROPOGATE_SETUP_TEMPLATE.substitute(configs)
    else:
        configs['feedback_propogate_defn'] = ""
        configs['feedback_propogate_setup'] = ""

    result = TEMPLATE.substitute(configs)

    output = open(output_path, 'w')
    output.write(result)
    output.close()

generate_function("add")
generate_function("and")
generate_function("if_expr")
generate_function("or")
generate_function("print")
generate_function("read_text_file")
generate_function("write_text_file")
