# API Documentation

This directory contains user-facing API documentation for NuoSTL.

## How to Use These Docs

- Each API page describes **purpose**, **public interface**, **behavior notes**, and **small C++ examples**.
- Code examples are intended to be minimal and readable; you can paste them into your own `.cpp` file and compile against the NuoSTL headers.
- When an API has important constraints (range limits, preconditions, exception behavior), those are listed near the top of the page.

## Structure

- `core/` (planned): core components that is similiar to STL (containers, algorithms, utilities).
- `extra/`: additional components that are not part of the core.

## Extra Components

- [Fraction](./extra/fraction.md): A class representing mathematical fractions with numerator and denominator, supporting arithmetic operations and normalization.
- [BigInteger](./extra/big_integer.md): An arbitrary precision signed integer with exact arithmetic, decimal text parsing and printing, and no fixed range limit.
- [Complex](./extra/complex.md): A complex number template with the arithmetic operators and mathematical functions of `std::complex`, plus near equality and exact integer powers.
- [Matrix](./extra/matrix.md): A dense matrix template with element access, arithmetic, transpose, trace, determinant, inverse and integer powers.