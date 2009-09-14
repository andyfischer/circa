
// The purpose of this test program is to experiment with handling integers as
// doubles. I'm thinking of storing all numbers in Circa as doubles (the way
// that Lua does it), and I'm trying to figure out if this will cause any
// problems.

// This program is a work in progress, right now it doesn't run so good.

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

typedef unsigned long long int64;

struct Double {
    int64 mantissa : 52;
    unsigned exp : 11;
    unsigned sign : 1;
};

struct Float {
    unsigned mantissa : 23;
    unsigned exp : 8;
    unsigned sign : 1;
};

Double to_struct_d(double d)
{
    return *((Double*) &d);
}

Float to_struct_f(float f)
{
    return *((Float*) &f);
}

std::string expand_d(double _d)
{
    Double d = to_struct_d(_d);
    std::stringstream out;
    out << _d << " (sign: " << d.sign << ", exp: " << d.exp << ", mant: " << d.mantissa << ")";
    return out.str();
}

std::string expand_f(float _f)
{
    Float f = to_struct_f(_f);
    std::stringstream out;
    out << _f << " (sign: " << f.sign << ", exp: " << f.exp << ", mant: " << f.mantissa << ")";
    return out.str();
}

bool is_integral_d(double d)
{
    return to_struct_d(d).exp == 1023; // this is not valid
}

std::vector<std::string> failures;

void fail(std::string const& msg)
{
    failures.push_back(msg);
}

int failure_count() { return (int) failures.size(); }
bool give_up() { return failure_count() > 50; }

int main(int i, char**c)
{
    // sanity checks

    for (int i=0; i < 0xffff; i++) {
        double d = i;

        if (!is_integral_d(d))
            fail("sanity check, not integral: " + expand_d(d));

        if (give_up())
            break;
    }

    // Display our success or failures
    if (failures.size() == 0) {
        std::cout << "Passed" << std::endl;
        return 0;
    } else {
        if (give_up())
            std::cout << "(failure limit reached)" << std::endl;

        std::cout << failures.size() << " failure(s):" << std::endl;
        for (size_t i=0; i < failures.size(); i++)
            std::cout << failures[i] << std::endl;
        return 1;
    }
}

