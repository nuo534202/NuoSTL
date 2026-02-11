# pragma once

#include <iostream>

#include "nuo_typedefs.hpp"

namespace nuostl
{

class Fraction
{
public:
    Fraction();
    Fraction(int64 up, int64 down);
    ~Fraction() = default;

public:
    Fraction& operator+(const Fraction& frac);
    Fraction& operator-(const Fraction& frac);
    Fraction& operator*(const Fraction& frac);
    Fraction& operator/(const Fraction& frac);
    Fraction& operator=(const Fraction& frac);
    bool operator==(const Fraction& frac) const;
    bool operator!=(const Fraction& frac) const;
    bool operator>(const Fraction& frac) const;
    bool operator>=(const Fraction& frac) const;
    bool operator<(const Fraction& frac) const;
    bool operator<=(const Fraction& frac) const;
    std::ostream& operator<< (const Fraction& frac) const;

    void SetUp(int64 up);
    void SetDown(int64 down);
    int64 GetUp() const;
    int64 GetDown() const;
    double GetFraction() const;

private:
    void Normalize();

private:
    /* TODO: use BigInteger when necessary */
    int64 up_;
    int64 down_;
};

}   /* namespace nuostl */