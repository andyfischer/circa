// Copyright 2008 Paul Hodge

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
struct Term;
struct Type;
struct ReferenceList;
struct ReferenceMap;

int& as_int(Term*);
float& as_float(Term*);
bool& as_bool(Term*);
std::string& as_string(Term*);
Term*& as_ref(Term*);


} // namespace circa

#endif
