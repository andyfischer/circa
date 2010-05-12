
# Named arguments #

Example:

    def get_distance(Point a, Point b)
        ...
    end

can be called with:

    get_distance(b: [1 0], a: [5 3])

or maybe:

    get_distance(b = [1 0], a = [5 3])

# Implementation notes #

In compiled code, the inputs should be arranged in the actual order appropriate for the
function, not necessarily in the order written. Reasons:
 - Faster for the executing function
 - Easier for introspection, avoids confusion
