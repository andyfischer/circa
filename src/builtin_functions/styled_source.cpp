// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

namespace circa {
namespace styled_source_function {

    void format_source(EvalContext*, Term* caller)
    {
        // TODO
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, format_source, "format_source(BranchRef b) -> StyledSource");
    }
}
}
