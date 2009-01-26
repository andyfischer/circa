// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

class Map {
private:
    struct Pair {
        Term* key;
        Term* value;

        Pair() : key(NULL), value(NULL) {}
    };

    // TODO: more efficient data structure
    std::vector<Pair> pairs;

public:

    Term* find(Term* key) {
        Pair *pair = findPair(key);
        if (pair == NULL) return NULL;
        return pair->value;
    }

    void assign(Term* key, Term* value) {
        Pair *existing = findPair(key);

        Term* duplicatedValue = create_value(NULL, value->type);
        duplicate_value(value, duplicatedValue);

        if (existing != NULL) {
            existing->value = duplicatedValue;
        } else {
            Term* duplicatedKey = create_value(NULL, key->type);
            duplicate_value(key, duplicatedKey);
            Pair newPair;
            newPair.key = duplicatedKey;
            newPair.value = duplicatedValue;
            pairs.push_back(newPair);
        }
    }

private:
    Pair* findPair(Term* key) {
        std::vector<Pair>::iterator it;
        for (it = pairs.begin(); it != pairs.end(); ++it) {
            if (values_equal(key, it->key))
                return &*it;
        }
        return NULL;
    }
};

} // namespace circa
