#pragma once

#include <stdint.h>

typedef struct BigNum {
  uint32_t capacity;
  uint64_t *components;
} BigNum;

BigNum bn_with_capacity(uint32_t);
void bn_clean(BigNum);
uint32_t bn_used_capacity(BigNum);
void bn_print(BigNum);
BigNum bn_add(BigNum, BigNum);
BigNum bn_sub(BigNum, BigNum);
BigNum bn_mul_karatsuba(BigNum *, BigNum *);
BigNum bn_fact_karatsuba(uint64_t);
BigNum bn_mul(BigNum, BigNum);
BigNum bn_fact(uint64_t);
BigNum bn_parse_hex(char *);
