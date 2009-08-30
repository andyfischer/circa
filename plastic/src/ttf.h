// Copyright 2009 Andrew Fischer

#ifndef CUTTLEFISH_TTF_INCLUDED
#define CUTTLEFISH_TTF_INCLUDED

namespace ttf {

// called before loading runtime.ca
void initialize(circa::Branch& branch);

// called after loading runtime.ca
void setup(circa::Branch& branch);

} // namespace ttf

#endif
