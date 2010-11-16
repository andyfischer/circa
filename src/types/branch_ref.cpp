// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace branch_ref_t {

    bool check_valid(EvalContext* cxt, Term* caller, TaggedValue* value)
    {
        if (!list_t::is_list(value)) {
            error_occurred(cxt, caller, "Input is not a BranchRef");
            return false;
        }

        TaggedValue* ref = value->getIndex(0);

        if (!is_ref(ref)) {
            error_occurred(cxt, caller, "Input is not a BranchRef (index 0 is not Ref)");
            return false;
        }
        
#if 0
        Term* target = as_ref(ref);

        if (!is_branch(target)) {
            error_occurred(cxt, caller, "Input is not a BranchRef (target is not Branch)");
            return false;
        }
#endif
        
        return true;
    }

    void set_from_ref(TaggedValue* value, Term* ref)
    {
        List* list = List::checkCast(value);
        touch(list);
        set_ref(list->getIndex(0), ref);
    }
    
    bool is_considered_config(Term* term)
    {
        if (term == NULL) return false;
        if (term->name == "") return false;
        if (!is_value(term)) return false;
        if (is_get_state(term)) return false;
        if (is_hidden(term)) return false;

        // ignore branch-based types
        if (is_branch(term)) return false;
        if (is_type(term)) return false;

        return true;
    }

    Branch& get_target_branch(TaggedValue* value)
    {
        List* list = List::checkCast(value);
        return list->get(0)->asRef()->nestedContents;
    }

    CA_FUNCTION(get_configs)
    {
        TaggedValue* value = INPUT(0);
        if (!check_valid(CONTEXT, CALLER, value))
            return;
        Branch& target_branch = get_target_branch(value);

        // One pass-through to get count
        int count = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;
            count++;
        }

        List* output = List::checkCast(OUTPUT);
        output->resize(count);

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;

            ca_assert(write < count);
            set_ref(output->get(write++), t);
        }
    }
    CA_FUNCTION(get_configs_nested)
    {
        TaggedValue* value = INPUT(0);
        if (!check_valid(CONTEXT, CALLER, value))
            return;

        Branch& target_branch = get_target_branch(value);
        List* output = List::checkCast(OUTPUT);

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

            if (write >= output->length())
                set_ref(output->append(), t);
            else
                set_ref(output->get(write), t);

            write++;
        }

        if (write < output->length())
            output->resize(write);
    }
    CA_FUNCTION(get_visible)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        Branch& output = as_branch(OUTPUT);

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

    CA_FUNCTION(get_relative_name)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        Term* target = INPUT(1)->asRef();

        if (target == NULL) {
            error_occurred(CONTEXT, CALLER, "term is NULL");
            return;
        }

        set_string(OUTPUT, get_relative_name(target_branch, target));
    }

    CA_FUNCTION(get_length)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        set_int(OUTPUT, target_branch.length());
    }

    CA_FUNCTION(get_index)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        int index = INT_INPUT(1);
        if (index >= target_branch.length())
            set_ref(OUTPUT, NULL);
        else
            set_ref(OUTPUT, target_branch[index]);
    }

    CA_FUNCTION(append_code)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        Branch& input = INPUT_TERM(1)->nestedContents;

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

    CA_FUNCTION(print_raw)
    {
        Branch& target_branch = get_target_branch(INPUT(0));

        set_string(OUTPUT, get_branch_raw(target_branch));
    }

    CA_FUNCTION(save)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        persist_branch_to_file(target_branch);
    }

    CA_FUNCTION(to_source)
    {
        Branch& target_branch = get_target_branch(INPUT(0));
        set_string(OUTPUT, get_branch_source_text(target_branch));
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
