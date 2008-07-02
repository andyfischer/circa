

# Kinds of blocks:
# 
# if
# if-else
# while
# for
#
# Each branch of a block has a namespace
# 
# Need to parse the inside of a block in the same way that we parse the outside
#
# Name resolution:
#   check local block
#   check parent block
#   etc
#   check highest block
#
# On call to 'createTerm', access the current branch
# Need to find out where a term is defined:
#   is it defined in current block?
#
# Keep track of stateful terms:
#   - when branch is finished, add an assign term


