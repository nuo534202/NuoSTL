#include "extra/fraction.hpp"

#include <stdexcept>

#include "utils/math.hpp"

namespace nuostl
{

Fraction::Fraction() : up_(0), down_(1) {}

Fraction::Fraction(int64 up, int64 down)
{
    if (down == 0)
        throw std::invalid_argument("Fraction: denominator cannot be 0!");

    up_ = up;
    down_ = down;

    Normalize();
}

Fraction& Fraction::operator+(const Fraction& frac)
{
    int128 lhs_up = static_cast<int128>(up_);
    int128 lhs_down = static_cast<int128>(down_);
    int128 rhs_up = static_cast<int128>(frac.up_);
    int128 rhs_down = static_cast<int128>(frac.down_);

    int128 new_up = lhs_up * rhs_down + lhs_down * rhs_up;
    int128 new_down = lhs_down * rhs_down;
    int64 up_down_gcd = gcd(new_up, new_down);

    up_ = static_cast<int64>(new_up / up_down_gcd);
    down_ = static_cast<int64>(new_down / up_down_gcd);

    return *this;
}

Fraction& Fraction::operator-(const Fraction& frac)
{
    int128 lhs_up = static_cast<int128>(up_);
    int128 lhs_down = static_cast<int128>(down_);
    int128 rhs_up = static_cast<int128>(frac.up_);
    int128 rhs_down = static_cast<int128>(frac.down_);

    int128 new_up = lhs_up * rhs_down - lhs_down * rhs_up;
    int128 new_down = lhs_down * rhs_down;
    int64 up_down_gcd = gcd(new_up, new_down);

    up_ = static_cast<int64>(new_up / up_down_gcd);
    down_ = static_cast<int64>(new_down / up_down_gcd);

    return *this;
}

Fraction& Fraction::operator*(const Fraction& frac)
{
    int128 lhs_up = static_cast<int128>(up_);
    int128 lhs_down = static_cast<int128>(down_);
    int128 rhs_up = static_cast<int128>(frac.up_);
    int128 rhs_down = static_cast<int128>(frac.down_);

    int128 new_up = lhs_up * rhs_up;
    int128 new_down = lhs_down * rhs_down;
    int64 up_down_gcd = gcd(new_up, new_down);

    up_ = static_cast<int64>(new_up / up_down_gcd);
    down_ = static_cast<int64>(new_down / up_down_gcd);

    return *this;
}

Fraction& Fraction::operator/(const Fraction& frac)
{
    int128 lhs_up = static_cast<int128>(up_);
    int128 lhs_down = static_cast<int128>(down_);
    int128 rhs_up = static_cast<int128>(frac.up_);
    int128 rhs_down = static_cast<int128>(frac.down_);

    int128 new_up = lhs_up * rhs_down;
    int128 new_down = lhs_down * rhs_up;
    int64 up_down_gcd = gcd(new_up, new_down);

    up_ = static_cast<int64>(new_up / up_down_gcd);
    down_ = static_cast<int64>(new_down / up_down_gcd);

    return *this;
}

Fraction& Fraction::operator=(const Fraction& frac)
{
    up_ = frac.up_;
    down_ = frac.down_;

    return *this;
}

bool Fraction::operator==(const Fraction& frac) const
{
    return (up_ == frac.up_ && down_ == frac.down_);
}

bool Fraction::operator!=(const Fraction& frac) const
{
    return (up_ != frac.up_ || down_ != frac.down_);
}

bool Fraction::operator>(const Fraction& frac) const
{
    int128 lhs = static_cast<int128>(up_) * static_cast<int128>(frac.down_);
    int128 rhs = static_cast<int128>(down_) * static_cast<int128>(frac.up_);

    return lhs > rhs;
}

bool Fraction::operator>=(const Fraction& frac) const
{
    int128 lhs = static_cast<int128>(up_) * static_cast<int128>(frac.down_);
    int128 rhs = static_cast<int128>(down_) * static_cast<int128>(frac.up_);

    return lhs >= rhs;
}

bool Fraction::operator<(const Fraction& frac) const
{
    int128 lhs = static_cast<int128>(up_) * static_cast<int128>(frac.down_);
    int128 rhs = static_cast<int128>(down_) * static_cast<int128>(frac.up_);

    return lhs < rhs;
}

bool Fraction::operator<=(const Fraction& frac) const
{
    int128 lhs = static_cast<int128>(up_) * static_cast<int128>(frac.down_);
    int128 rhs = static_cast<int128>(down_) * static_cast<int128>(frac.up_);

    return lhs <= rhs;
}

void Fraction::SetUp(int64 up)
{
    up_ = up;
}

void Fraction::SetDown(int64 down)
{
    if (down == 0)
        throw std::invalid_argument("Fraction: denominator cannot be 0!");

    down_ = down;
}

int64 Fraction::GetUp() const
{
    return up_;
}

int64 Fraction::GetDown() const
{
    return down_;
}

double Fraction::GetFraction() const
{
    return ((double) up_) / down_;
}

void Fraction::Normalize()
{
    if (down_ < 0)
    {
        up_ = -up_;
        down_ = -down_;
    }
}

std::ostream& operator<< (std::ostream& cout, const Fraction& frac)
{
    cout << frac.GetUp() << "/" << frac.GetDown();
    return cout;
}

}   /* namespace nuostl */