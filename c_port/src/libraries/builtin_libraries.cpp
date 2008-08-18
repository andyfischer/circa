
#include <circa.h>

#include "graphviz.h"

namespace circa {

void initialize_external_libraries(Branch* kernel)
{
    graphviz_initialize(kernel);
}

}
