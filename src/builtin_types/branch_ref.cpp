// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_ref_t {

    void set_from_ref(TaggedValue* value, Term* ref)
    {
#ifdef NEWLIST
        List* list = (List*) value;
        mutate(list);
        make_ref(list->getIndex(0), ref);
#else
        Branch& branch = as_branch(value);
        branch[0]->asRef() = ref;
#endif
    }
    
    bool is_considered_config(Term* term)
    {
        if (term == NULL) return false;
        if (term->name == "") return false;
        if (!is_value(term)) return false;
        if (is_stateful(term)) return false;
        if (is_hidden(term)) return false;

        // ignore branch-based types
        if (is_branch(term)) return false;
        if (is_type(term)) return false;

        return true;
    }

    Branch& get_target_branch(TaggedValue* value)
    {
#ifdef NEWLIST
        List* list = (List*) value;
        return as_branch(list->get(0)->asRef());
#else
        Branch& branch = as_branch(value);
        return branch[0]->asRef()->asBranch();
#endif
    }

    void get_configs(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));

#ifdef NEWLIST
        // One pass through to get count
        int count = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;
            count++;
        }

        List* output = (List*) caller;
        output->resize(count);

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;

            assert(write < count);
            make_ref(output->get(write++), t);
        }
#else
        Branch& output = caller->asBranch();

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
#endif
    }
    void get_configs_nested(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        Branch& output = caller->asBranch();

        int write = 0;
        for (BranchIterator it(target_branch); !it.finished(); it.advance()) {
            Term* t = *it;

            if (is_branch(t)) {
                // check if we should explore this branch
                bool explore = (t->type == CODE_TYPE || is_function(t))
                        && !is_hidden(t) && t->name != "";

                if (!explore)
                    it.skipNextBranch();
                
                continue;
            }

            if (!is_considered_config(t))
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
    }
    void get_visible(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        Branch& output = caller->asBranch();

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (is_hidden(t) || t->function == COMMENT_FUNC)
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
    }

    void get_relative_name(EvalContext* cxt, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        Term* target = caller->input(1)->asRef();

        if (target == NULL) {
            error_occurred(cxt, caller, "term is NULL");
            return;
        }

        set_str(caller, get_relative_name(target_branch, target));
    }

    void get_length(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        set_int(caller, target_branch.length());
    }
    void get_index(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        int index = int_input(caller, 1);
        if (index >= target_branch.length())
            set_ref(caller, NULL);
        else
            set_ref(caller, target_branch[index]);
    }
    void append_code(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        Branch& input = as_branch(caller->input(1));

        if (input.length() == 0)
            return;

        int previousLast = target_branch.length();

        Branch temp_copy;
        duplicate_branch(input, temp_copy);
        lift_closure(temp_copy);

        duplicate_branch(temp_copy, target_branch);

        // Strip trailing whitespace after the formally-last and newly-last terms
        // so that the resulting source looks better.
        if (previousLast > 0)
            target_branch[previousLast-1]->removeProperty("syntax:lineEnding");
        target_branch[target_branch.length()-1]->removeProperty("syntax:lineEnding");
    }
    void print_raw(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));

        set_str(caller, get_branch_raw(target_branch));
    }
    void save(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        persist_branch_to_file(target_branch);
    }
    void to_source(EvalContext*, Term* caller)
    {
        Branch& target_branch = get_target_branch(caller->input(0));
        set_str(caller, get_branch_source_text(target_branch));
    }

    void setup_type(Type* type)
    {
        import_member_function(type, branch_ref_t::get_configs,
            "get_configs(BranchRef) -> List");
        import_member_function(type, branch_ref_t::get_configs_nested,
            "get_configs_nested(BranchRef) -> List");
        import_member_function(type, branch_ref_t::get_relative_name,
            "get_relative_name(BranchRef, Ref) -> string");
        import_member_function(type, branch_ref_t::get_visible,
            "get_visible(BranchRef) -> List");
        import_member_function(type, branch_ref_t::get_length,
            "length(BranchRef) -> int");
        import_member_function(type, branch_ref_t::get_index,
            "get_index(BranchRef, int) -> Ref");
        import_member_function(type, branch_ref_t::append_code,
            "append_code(BranchRef, Branch)");
        import_member_function(type, branch_ref_t::print_raw,
            "print_raw(BranchRef) -> string");
        import_member_function(type, branch_ref_t::save,
            "save(BranchRef)");
        import_member_function(type, branch_ref_t::to_source,
            "to_source(BranchRef) -> string");
    }

} // namespace branch_ref_t
} // namespace circa
