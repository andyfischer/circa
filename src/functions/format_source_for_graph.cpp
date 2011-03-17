// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

namespace circa {
namespace format_source_for_graph_function {
    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(format_source_for_graph,
        "format_source_for_graph(BranchRef br) -> List")
    {
        /*
        PhraseLink {
            Point startPos
            Point endPos
            Ref source
            Ref user
        }

        Phrase {
            string text
            Ref term
            int phrase_type
            PhraseLink[] inputs
            PhraseLink[] users

        }
        Statement {
            Phrase[] phrases
            Rect position
            Ref term
        }
        */

        Branch& branch = *as_branch_ref(INPUT(0));
        List& statementList = *List::cast(OUTPUT, 0);

        if (branch.length() == 0)
            return;

        int topleftSourceX = branch[0]->sourceLoc.col;
        int topleftSourceY = branch[0]->sourceLoc.line;

        // Initialize the Statement list
        for (int i=0; i < branch.length(); i++) {
            Term* term = branch[i];
            if (!is_statement(term))
                continue;

            List& statement = *List::cast(statementList.append(), 3);

            // item[0]: Phrase[] phrases
            List& phrases = *List::cast(statement[0], 0);

            // Start the 'phrases' list using regular term formatting
            StyledSource styled;
            format_term_source(&styled, term);
            swap(&styled._phrases, &phrases);

            // For each phrase, we'll append 'inputs' and 'users'
            for (int i=0; i < phrases.length(); i++) {
                List& phrase = *List::checkCast(phrases[i]);
                phrase.resize(5);
                set_list(phrase[3], 0);
                set_list(phrase[4], 0);
            }

            // item[1]: Rect position
            List& position = *List::cast(statement[2], 4);
            set_int(position[0], term->sourceLoc.col - topleftSourceX);
            set_int(position[1], term->sourceLoc.line - topleftSourceY);
            set_int(position[2], term->sourceLoc.colEnd - topleftSourceX);
            set_int(position[3], term->sourceLoc.lineEnd - topleftSourceY);

            // item[2]: Ref term
            set_ref(statement[2], term);
        }
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
