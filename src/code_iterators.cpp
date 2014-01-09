// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "inspection.h"
#include "term.h"

namespace circa {

BlockIterator::BlockIterator(Block* block, bool backwards)
  : _backwards(backwards),
    _skipNextBlock(false)
{
    reset(block);
}

void BlockIterator::reset(Block* block)
{
    _stack.clear();
    if (block->length() != 0) {
        int firstIndex = _backwards ? block->length() - 1 : 0;
        _stack.push_back(IteratorFrame(block, firstIndex));
    }
}

bool BlockIterator::finished()
{
    return _stack.empty();
}

Term* BlockIterator::current()
{
    ca_assert(!finished());
    IteratorFrame& frame = _stack.back();
    return frame.block->get(frame.index);
}

void BlockIterator::advance()
{
    ca_assert(!finished());
    if (_skipNextBlock) {
        _skipNextBlock = false;
        advanceSkippingBlock();
        return;
    }

    Term* term = current();

    // Check to start an inner block.
    if (term && term->nestedContents && term->contents()->length() > 0) {
        Block* contents = nested_contents(term);
        int firstIndex = _backwards ? contents->length() - 1 : 0;
        _stack.push_back(IteratorFrame(contents, firstIndex));
        return;
    }

    // Otherwise, just advance. PS, it's not really accurate to say that we are "skipping"
    // any blocks, because we just checked if there was one.
    advanceSkippingBlock();
}

void BlockIterator::advanceSkippingBlock()
{
    while (true) {
        Block* block = _stack.back().block;
        int& index = _stack.back().index;

        index += _backwards ? -1 : 1;

        if (index < 0 || index >= block->length())
            _stack.pop_back();
        else
            break;

        if (_stack.empty())
            break;
    }
}

int BlockIterator::depth()
{
    return (int) _stack.size() - 1;
}

// UpwardsIterator

UpwardIterator::UpwardIterator(Term* startingTerm)
  : _stopAt(NULL)
{
    _block = startingTerm->owningBlock;
    _index = startingTerm->index;

    // advance once, we don't yield the starting term
    advance();
}

void
UpwardIterator::stopAt(Block* block)
{
    _stopAt = block;
}

bool UpwardIterator::finished()
{
    return _block == NULL;
}

Term* UpwardIterator::current()
{
    ca_assert(_block != NULL);
    return _block->get(_index);
}

void UpwardIterator::advance()
{
    do {
        _index--;

        if (_index < 0) {
            Term* parentTerm = _block->owningTerm;
            if (parentTerm == NULL) {
                _block = NULL;
                return;
            }

            _block = parentTerm->owningBlock;
            _index = parentTerm->index;

            if (_block == _stopAt)
                _block = NULL;
            if (_block == NULL)
                return;
        }
        
        // repeat until we find a non NULL term
    } while (unfinished() && current() == NULL);
}

UpwardIterator2::UpwardIterator2()
  : block(NULL), index(0), lastBlock(NULL)
{
}

UpwardIterator2::UpwardIterator2(Term* firstTerm)
  : lastBlock(NULL)
{
    block = firstTerm->owningBlock;
    index = firstTerm->index;
}

UpwardIterator2::UpwardIterator2(Block* startingBlock)
  : lastBlock(NULL)
{
    block = startingBlock;
    index = block->length() - 1;
    advanceWhileInvalid();
}

void UpwardIterator2::stopAt(Block* _lastBlock)
{
    lastBlock = _lastBlock;
}

bool UpwardIterator2::finished()
{
    return block == NULL;
}

Term* UpwardIterator2::current()
{
    ca_assert(!finished());
    return block->get(index);
}

void UpwardIterator2::advance()
{
    index--;
    advanceWhileInvalid();
}

void UpwardIterator2::advanceWhileInvalid()
{
possibly_invalid:
    if (finished())
        return;

    if (index < 0) {
        // Stop if we've finished the lastBlock.
        if (block == lastBlock) {
            block = NULL;
            return;
        }

        Block* previousBlock = block;
        block = get_parent_block(block);
        Term* parentTerm = get_parent_term(previousBlock);

        if (block == NULL || parentTerm == NULL) {
            block = NULL;
            return;
        }

        index = parentTerm->index - 1;

        goto possibly_invalid;
    }
}

BlockIteratorFlat::BlockIteratorFlat(Block* blockIn)
 : block(blockIn), index(0)
{
    advanceWhileInvalid();
}

bool BlockIteratorFlat::finished()
{
    return index >= block->length();
}
void BlockIteratorFlat::advance()
{
    index++;
    advanceWhileInvalid();
}
void BlockIteratorFlat::advanceWhileInvalid()
{
possibly_invalid:

    if (finished())
        return;

    if (block->get(index) == NULL) {
        index++;
        goto possibly_invalid;
    }
}
Term* BlockIteratorFlat::current()
{
    return block->get(index);
}

BlockInputIterator::BlockInputIterator(Block* blockIn)
 : block(blockIn)
{
    index = 0;
    inputIndex = 0;
    advanceWhileInvalid();
}

bool
BlockInputIterator::finished()
{
    return index >= block->length();
}

void BlockInputIterator::advance()
{
    inputIndex++;
    advanceWhileInvalid();
}

void BlockInputIterator::advanceWhileInvalid()
{
possibly_invalid:

    if (finished())
        return;

    Term* current = block->get(index);

    if (current == NULL) {
        index++;
        inputIndex = 0;
        goto possibly_invalid;
    }

    if (inputIndex >= current->numInputs()) {
        index++;
        inputIndex = 0;
        goto possibly_invalid;
    }

    if (current->input(inputIndex) == NULL) {
        inputIndex++;
        goto possibly_invalid;
    }
}

Term* BlockInputIterator::currentTerm()
{
    return block->get(index);
}

Term* BlockInputIterator::currentInput()
{
    return block->get(index)->input(inputIndex);
}

int BlockInputIterator::currentInputIndex()
{
    return inputIndex;
}

OuterInputIterator::OuterInputIterator(Block* block)
 : blockInputIterator(block)
{
    advanceWhileInvalid();
}

bool
OuterInputIterator::finished()
{
    return blockInputIterator.finished();
}

void OuterInputIterator::advance()
{
    blockInputIterator.inputIndex++;
    advanceWhileInvalid();
}

void OuterInputIterator::advanceWhileInvalid()
{
possibly_invalid:
    blockInputIterator.advanceWhileInvalid();

    if (finished())
        return;

    // Only stop on outer inputs
    if (blockInputIterator.currentInput()->owningBlock == blockInputIterator.block) {
        blockInputIterator.inputIndex++;
        goto possibly_invalid;
    }
}

Term* OuterInputIterator::currentTerm()
{
    return blockInputIterator.currentTerm();
}

Term* OuterInputIterator::currentInput()
{
    return blockInputIterator.currentInput();
}

int OuterInputIterator::currentInputIndex()
{
    return blockInputIterator.currentInputIndex();
}

BlockIterator2::BlockIterator2()
{
    _current = NULL;
    _topBlock = NULL;
}
BlockIterator2::BlockIterator2(Block* block)
{
    _current = block->get(0);
    _topBlock = block;
}

void BlockIterator2::startAt(Term* term)
{
    if (term == NULL) {
        _current = NULL;
        _topBlock = NULL;
        return;
    }

    _current = term;
    _topBlock = term->owningBlock;
}

void BlockIterator2::advance()
{
    // Possibly iterate through the contents of this block.
    if (has_nested_contents(_current) && nested_contents(_current)->length() > 0) {
        _current = nested_contents(_current)->get(0);
        return;
    }

    // Advance to next index.
    Block* block = _current->owningBlock;
    int index = _current->index + 1;

    // Possibly loop as we pop out of finished blocks.
    possibly_invalid:

    if (index >= block->length() || block == NULL) {
        // Finished this block.

        if (block == _topBlock || block == NULL) {
            // Finished the iteration.
            _current = NULL;
            _topBlock = NULL;
            return;
        }

        // Advance to the next term in the parent block.
        Term* parentTerm = block->owningTerm;
        if (parentTerm == NULL) {
            // No block parent. It's weird that we hit this case before we reached
            // the topBlock, but anyway, finish the iteration.
            _current = NULL;
            _topBlock = NULL;
            return;
        }

        block = parentTerm->owningBlock;
        index = parentTerm->index + 1;
        goto possibly_invalid;
    }

    // Skip over NULL terms.
    if (block->get(index) == NULL) {
        index++;
        goto possibly_invalid;
    }

    // Index is valid. Save the position as a Term*.
    _current = block->get(index);
}

NameVisibleIterator::NameVisibleIterator(Term* term)
{
    _target = term;

    // Start at the term immediately after the target.
    _iterator.startAt(following_term(_target));
}


void NameVisibleIterator::advance()
{
    _iterator.advance();

    if (_iterator.finished())
        return;

    // If we reach a term with the same name binding as our target, then stop.
    // Our target term is no longer visible.
    if (equals(&_iterator.current()->nameValue, &_target->nameValue)) {
        _iterator.stop();
        return;
    }
}

} // namespace circa
