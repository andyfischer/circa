// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

struct Map {
    struct Pair {
        Term* key;
        Term* value;

        Pair() : key(NULL), value(NULL) {}
    };

    // TODO: more efficient data structure
    std::vector<Pair> _pairs;

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
            _pairs.push_back(newPair);
        }
    }

    Pair* findPair(Term* key) {
        std::vector<Pair>::iterator it;
        for (it = _pairs.begin(); it != _pairs.end(); ++it) {
            if (values_equal(key, it->key))
                return &*it;
        }
        return NULL;
    }
};

class MapIterator
{
    Map* _map;
    int _pairIndex;

public:
    MapIterator(Map* map)
      : _map(map), _pairIndex(0)
    {
        if (_map->_pairs.size() == 0)
            _map = NULL;
    }

    Map::Pair& current()
    {
        assert(!finished());
        return _map->_pairs[_pairIndex];
    }

    void advance()
    {
        assert(!finished());
        _pairIndex++;
        if (_map->_pairs.size() == 0)
            _map = NULL;
    }

    bool finished()
    {
        return _map == NULL;
    }
};

/*
class MapExternalPointerIterator : PointerIterator
{
    MapIterator _iterator;

public:
    MapExternalPointerIterator(Map* map)
      : _iterator(map)
    {
    }

    Term* current()
};
*/

} // namespace circa
