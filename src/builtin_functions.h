// Copyright 2008 Andrew Fischer

#ifndef CIRCA_BUILTIN_FUNCTIONS_INCLUDED
#define CIRCA_BUILTIN_FUNCTIONS_INCLUDED

namespace circa {

typedef void (*RegistrationFunction)(Branch& kernel);

extern std::vector<RegistrationFunction> gRegistrationFunctions;

class RegisterBuiltinFunctionOnStartupHack
{
public:
    RegisterBuiltinFunctionOnStartupHack(RegistrationFunction func);
};

#define REGISTER_BUILTIN_FUNCTION(n) static RegisterBuiltinFunctionOnStartupHack #n;

void registerBuiltinFunctions(Branch& kernel);

} // namespace circa

#endif
