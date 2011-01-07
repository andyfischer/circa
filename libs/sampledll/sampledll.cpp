
#include <circa.h>
#include <importing_macros.h>

using namespace circa;

extern "C" {

void on_load(Branch* branch)
{
    std::cout << "sampledll loaded" << std::endl;
}

CA_FUNCTION(sample_a)
{
    set_string(OUTPUT, "it works");
}

CA_FUNCTION(ns__concat)
{
    set_string(OUTPUT, std::string(STRING_INPUT(0)) + STRING_INPUT(1));
}

}
