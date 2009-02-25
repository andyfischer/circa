// Copyright 2008 Andrew Fischer

#ifndef CIRCA_COMMON_HEADERS_INCLUDED
#define CIRCA_COMMON_HEADERS_INCLUDED

#include <cassert>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

namespace circa {

struct Branch;
struct Function;
struct PointerIterator;
struct Term;
struct Type;
struct Ref;
struct RefList;
struct ReferenceMap;

int& as_int(Term*);
float& as_float(Term*);
bool& as_bool(Term*);
std::string& as_string(Term*);
Ref& as_ref(Term*);

// Build options
#define TRACK_USERS 0

} // namespace circa

#endif
