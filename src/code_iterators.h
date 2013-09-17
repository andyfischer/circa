// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include <vector>

namespace circa {

struct BlockIterator
{
    struct IteratorFrame {
        Block* block;
        int index;

        IteratorFrame(Block* b, int i) : block(b), index(i) {}
    };

    std::vector<IteratorFrame> _stack;
    bool _backwards;
    bool _skipNextBlock;

    BlockIterator(Block* block, bool backwards=false);
    void reset(Block* block);
    bool finished();
    bool unfinished() { return !finished(); }
    Term* current();
    void advance();
    void advanceSkippingBlock();
    int depth();

    // Next call to advance() will not explore a block, if there is one.
    void skipNextBlock() { _skipNextBlock = true; }

    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};

struct UpwardIterator
{
    Block* _block;
    int _index;

    Block* _stopAt;

    UpwardIterator(Term* startingTerm);

    // stopAt is optional, if set then we will not continue past the given block.
    void stopAt(Block* block);

    bool finished();
    Term* current();
    void advance();

    bool unfinished() { return !finished(); }
    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};

struct UpwardIterator2
{
    Block* block;
    int index;
    Block* lastBlock;

    UpwardIterator2();
    UpwardIterator2(Term* firstTerm);
    UpwardIterator2(Block* startingBlock);
    void stopAt(Block* _lastBlock);
    bool finished();
    Term* current();
    void advance();
    void advanceWhileInvalid();
    operator bool() { return !finished(); }
    void operator++() { advance(); }
};

struct BlockIteratorFlat
{
    Block* block;
    int index;

    BlockIteratorFlat(Block* block);

    bool finished();
    void advance();
    void advanceWhileInvalid();

    Term* current();

    Term* operator*() { return current(); }
    bool unfinished() { return !finished(); }
    void operator++() { advance(); }
};

// This iterator steps over each input of each term in the block. It will skip
// over any NULL terms, and it will skip over NULL inputs.
struct BlockInputIterator
{
    Block* block;
    int index;
    int inputIndex;

    BlockInputIterator(Block* block);

    bool finished();
    void advance();
    void advanceWhileInvalid();

    Term* currentTerm();
    Term* currentInput();
    int currentInputIndex();

    bool unfinished() { return !finished(); }
    void operator++() { advance(); }
    operator bool() { return !finished(); }
};

// This iterator is a filtered version of BlockInputIterator. It will only return
// "outer inputs": inputs to terms that are outside of the provided block.
struct OuterInputIterator
{
    BlockInputIterator blockInputIterator;

    OuterInputIterator(Block* block);

    bool finished();
    void advance();
    void advanceWhileInvalid();

    Term* currentTerm();
    Term* currentInput();
    int currentInputIndex();

    bool unfinished() { return !finished(); }
    void operator++() { advance(); }
};

struct BlockIterator2
{
    Term* _current;
    Block* _topBlock;

    BlockIterator2();
    BlockIterator2(Block* block);
    void startAt(Term* term);

    Term* current() { return _current; }
    void advance();
    bool finished() { return _current == NULL; }
    bool unfinished() { return !finished(); }
    void stop() { _current = NULL; _topBlock = NULL; }

    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};

struct NameVisibleIterator
{
    // For a term T, iterates across all the terms where T is name-visible.

    BlockIterator2 _iterator;
    Term* _target;

    NameVisibleIterator(Term* term);

    Term* current() { return _iterator.current(); }
    void advance();
    bool finished() { return _iterator.finished(); }
    bool unfinished() { return !finished(); }

    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};

} // namespace circa
