#ifndef CIRCA__TERM_LIST__INCLUDED
#define CIRCA__TERM_LIST__INCLUDED

class Branch;
class Type;
class Term;

struct TermList
{
    std::vector<Term*> items;

    explicit TermList() { }

    // Convenience constructors
    explicit TermList(Term* term) {
        items.push_back(term);
    }
    TermList(Term* term1, Term* term2) {
        items.push_back(term1);
        items.push_back(term2);
    }
    TermList(Term* term1, Term* term2, Term* term3) {
        items.push_back(term1);
        items.push_back(term2);
        items.push_back(term3);
    }

    int count() const;
    void append(Term* term);
    void setAt(int index, Term* term);
    void clear();
    Term* get(int index) const;
    Term* operator[](int index) const;
};


#endif
