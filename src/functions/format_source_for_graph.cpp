// Copyright (c) 2007-2010 Andrew Fischer. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace format_source_for_graph_function {
    CA_START_FUNCTIONS;

    struct FragmentLocation {
        int x;
        int y;
        int length;
        FragmentLocation() {}
        FragmentLocation(int _x, int _y, int _l) : x(_x), y(_y), length(_l) {}
    };

    CA_DEFINE_FUNCTION(format_source_for_graph,
        "format_source_for_graph(Branch br) -> List")
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
            Rect location

        }
        Statement {
            Phrase[] phrases
            Rect location
            Ref term
            PhraseLink[] inputs
            PhraseLink[] users
        }
        */

        Branch* branch = as_branch(INPUT(0));
        List& statementList = *List::cast(OUTPUT, 0);

        if (branch->length() < 2)
            return;

        // this is a hacky way to get the topleft point for the branch:
        int topleftSourceX = branch->get(1)->sourceLoc.col;
        int topleftSourceY = branch->get(1)->sourceLoc.line;

        std::map<Term*, int> termToStatement;
        std::map<Term*, FragmentLocation> termToFunctionCallLocation;

        // Initialize the Statement list
        for (int i=0; i < branch->length(); i++) {
            Term* term = branch->get(i);
            if (!is_statement(term))
                continue;

            int statementIndex = statementList.length();
            termToStatement[term] = statementIndex;

            List& statement = *List::cast(statementList.append(), 5);

            // item[3]: PhraseLink[] inputs
            // item[4]: PhraseLink[] users
            List& statementInputs = *List::cast(statement[3], 0);
            //List& statementUsers = *List::cast(statement[4], 0);

            // item[0]: Phrase[] phrases
            List& phrases = *List::cast(statement[0], 0);

            // Start the 'phrases' list using regular term formatting
            StyledSource styled;
            format_term_source(&styled, term);
            swap(&styled._phrases, &phrases);

            // For each phrase, we'll append 'location'

            int currentX = term->sourceLoc.col - topleftSourceX;
            int currentY = term->sourceLoc.line - topleftSourceY;

            for (int i=0; i < phrases.length(); i++) {
                List& phrase = *List::checkCast(phrases[i]);
                Term* term = as_ref(phrase[1]);
                int phrase_type = as_int(phrase[2]);

                termToStatement[term] = statementIndex;

                phrase.resize(4);

                int length = as_string(phrase[0]).size();

                List& location = *List::cast(phrase[3], 4);
                set_int(location[0], currentX);
                set_int(location[1], currentY);
                set_int(location[2], currentX + length);
                set_int(location[3], currentY);


                if (phrase_type == phrase_type::FUNCTION_NAME)
                    termToFunctionCallLocation[term] = FragmentLocation(currentX, currentY, length);

                if (as_string(phrase[0]) == "\n") {
                    currentX = 0;
                    currentY++;
                } else {
                    currentX += length;
                }
            }

            // For each input, add a PhraseLink
            for (int i=0; i < term->numInputs(); i++) {
                Term* input = term->input(i);
                if (input == NULL)
                    continue;

                if (termToStatement.find(term) == termToStatement.find(input))
                    continue;

                List& phraseLink = *List::cast(statementInputs.append(), 4);

                Point& startPos = *Point::cast(phraseLink[0]);
                Point& endPos = *Point::cast(phraseLink[1]);

                set_ref(phraseLink[2], input);
                set_ref(phraseLink[3], term);

                if (termToFunctionCallLocation.find(input) != termToFunctionCallLocation.end()) {

                    FragmentLocation& inputLoc = termToFunctionCallLocation[input];
                    startPos.set(inputLoc.x + inputLoc.length / 2.0, inputLoc.y);
                } else {
                    startPos.set(
                        (input->sourceLoc.col + input->sourceLoc.colEnd) / 2.0 - topleftSourceX,
                        input->sourceLoc.line - topleftSourceY);
                }

                if (termToFunctionCallLocation.find(term) != termToFunctionCallLocation.end()) {
                    FragmentLocation& userLoc = termToFunctionCallLocation[term];
                    endPos.set(userLoc.x + userLoc.length / 2.0, userLoc.y);
                } else {
                    endPos.set((term->sourceLoc.col + term->sourceLoc.colEnd) / 2.0 - topleftSourceX,
                        term->sourceLoc.line - topleftSourceY);
                }
            }

            // item[1]: Rect location
            List& location = *List::cast(statement[1], 4);
            set_int(location[0], term->sourceLoc.col - topleftSourceX);
            set_int(location[1], term->sourceLoc.line - topleftSourceY);
            set_int(location[2], term->sourceLoc.colEnd - topleftSourceX + 1);
            set_int(location[3], term->sourceLoc.lineEnd - topleftSourceY);

            // item[2]: Ref term
            set_ref(statement[2], term);
        }
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
