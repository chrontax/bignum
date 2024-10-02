# Docs

## `struct BigNum`

Contains its component count in `capacity` and a pointer to them in `components`.

## `bn_with_capacity`

Takes the desired capacity and constructs a BigNum allocating an array of
such size and initializes all components to 0.

If the capacity is 0, `components` will be `NULL`.

## `bn_clean`

Takes a BigNum and frees the component buffer.

## `bn_used_capacity`

Takes a BigNum and returns the minimum capacity required to store the number
without leading zeroes.

## `bn_add`

Returns the sum of its operands.

## `bn_sub`

Returns the difference of its operands.

### Underflow

If the first operand is smaller than the second, the result's last component
will have underflowed to `UINT64_MAX`.

## `bn_mul`

Returns the product of its operands.

## `bn_fact`

Convenience function for calculating a factorial.

## `bn_parse_hex`

Parses a hexadecimal string and returns an equivalent BigNum.

If the string is `NULL`, has a length of 0, or contains a character that is not
a hexadecimal digit, the result will be `bn_with_capacity(0)`.

## `bn_print`

Prints the BigNum in hexadecimal with a 0x prefix, without leading zeroes and
a new line.

## `bn_mul_karatsuba`

Returns the product of its operands using the Karatsuba algorithm.

In theory this should perform better, however it seems my implementation is not
up to par, since `bn_mul`, which uses long multiplication is faster.

## `bn_fact_karatsuba`

Convenience function for calculating a factorial. Instead of `bn_mul`
uses `bn_mul_karatsuba`.
