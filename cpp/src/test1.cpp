

#include "Builtins.h"
#include "CodeUnit.h"
#include "CommonHeaders.h"

int main(char * args[])
{
   builtins::bootstrap_kernel();
   builtins::bootstrap_builtins();

   Term* a_number = codeunit::create_constant(builtins::BUILTINS,
         builtins::INT_TYPE);

   int_type::set_data(a_number, 5);

   std::cout << int_type::to_string(a_number) << std::endl;
}
