#include "bignum.h"
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(x, y) (((x) >= (y)) ? (x) : (y))

BigNum bn_with_capacity(uint32_t capacity) {
  BigNum bn = {capacity, capacity ? malloc(sizeof(uint64_t) * capacity) : NULL};
  for (int i = 0; i < capacity; i++)
    bn.components[i] = 0;
  return bn;
}

void bn_clean(BigNum bn) { free(bn.components); }

uint32_t bn_used_capacity(BigNum bn) {
  if (!bn.capacity || !bn.components)
    return 0;

  for (int i = bn.capacity - 1; i >= 0; i--)
    if (bn.components[i])
      return i + 1;

  return 1;
}

void bn_print(BigNum bn) {
  printf("0x");
  uint32_t used = bn_used_capacity(bn);
  if (!used) {
    printf("0\n");
    return;
  }
  printf("%lx", bn.components[used - 1]);
  for (int i = 2; i <= used; i++)
    printf("%016lx", bn.components[used - i]);
}

BigNum bn_add(BigNum a, BigNum b) {
  uint32_t a_used = bn_used_capacity(a);
  uint32_t b_used = bn_used_capacity(b);
  BigNum c = bn_with_capacity(max(a_used, b_used) + 1);

  for (int i = 0; i < a_used; i++)
    c.components[i] = a.components[i];

  for (int i = 0; i < b_used; i++) {
    int carry = (c.components[i] += b.components[i]) < b.components[i];

    int k = 1;
    while (carry) {
      c.components[i + k]++;
      carry = !c.components[i + k];
      k++;
    }
  }

  return c;
}

BigNum bn_sub(BigNum a, BigNum b) {
  uint32_t a_used = bn_used_capacity(a);
  uint32_t b_used = bn_used_capacity(b);
  BigNum c = bn_with_capacity(max(a_used, b_used) + 1);

  for (int i = 0; i < a_used; i++)
    c.components[i] = a.components[i];

  for (int i = 0; i < b_used; i++) {
    if (c.components[i] < b.components[i])
      for (int j = i + 1; j < max(a_used, b_used) + 1 && !(c.components[j]--);
           j++)
        ;

    c.components[i] -= b.components[i];
  }

  return c;
}

BigNum bn_mul_karatsuba(BigNum *a, BigNum *b) {
  uint32_t a_used = bn_used_capacity(*a);
  uint32_t b_used = bn_used_capacity(*b);

  if (a_used == 1 && b_used == 1) {
    BigNum result = bn_with_capacity(2);

    asm("mulq %%rbx;"
        : "=a"(result.components[0]), "=d"(result.components[1])
        : "a"(a->components[0]), "b"(b->components[0]));

    return result;
  }

  if (a_used > b_used) {
    b->components = realloc(b->components, a_used * sizeof(uint64_t));
    b->capacity = a_used;
    for (int i = b_used; i < a_used; i++)
      b->components[i] = 0;
  } else if (b_used > a_used) {
    a->components = realloc(a->components, b_used * sizeof(uint64_t));
    a->capacity = b_used;
    for (int i = a_used; i < b_used; i++)
      a->components[i] = 0;
  }
  uint32_t max_used = max(a_used, b_used);
  uint32_t split = max_used / 2;

  BigNum *tmp_p[6];
  BigNum tmp[8];

  BigNum a0 = bn_with_capacity(split);
  tmp_p[0] = &a0;
  BigNum b0 = bn_with_capacity(split);
  tmp_p[1] = &b0;

  for (int i = 0; i < split; i++) {
    a0.components[i] = a->components[i];
    b0.components[i] = b->components[i];
  }

  BigNum a1 = bn_with_capacity(max_used - split);
  tmp_p[2] = &a1;
  BigNum b1 = bn_with_capacity(max_used - split);
  tmp_p[3] = &b1;

  for (int i = split; i < max_used; i++) {
    a1.components[i - split] = a->components[i];
    b1.components[i - split] = b->components[i];
  }

  BigNum z0 = tmp[0] = bn_mul_karatsuba(&a0, &b0);

  BigNum sum0 = bn_add(a0, a1);
  BigNum sum1 = bn_add(b0, b1);
  tmp_p[4] = &sum0;
  tmp_p[5] = &sum1;
  BigNum z1 = tmp[1] = bn_mul_karatsuba(&sum0, &sum1);

  BigNum z2 = tmp[2] = bn_mul_karatsuba(&a1, &b1);

  BigNum moved_z2 = tmp[3] = bn_with_capacity(split * 2 + bn_used_capacity(z2));
  for (int i = split * 2; i < split * 2 + bn_used_capacity(z2); i++)
    moved_z2.components[i] = z2.components[i - split * 2];
  BigNum res2 = tmp[6] = bn_sub(tmp[5] = bn_sub(z1, z2), z0);
  BigNum res1 = tmp[4] = bn_with_capacity(split + bn_used_capacity(res2));
  for (int i = split; i < split + bn_used_capacity(res2); i++)
    res1.components[i] = res2.components[i - split];

  BigNum result = bn_add(tmp[7] = bn_add(moved_z2, res1), z0);

  for (int i = 0; i < 6; i++)
    bn_clean(*tmp_p[i]);
  for (int i = 0; i < 8; i++)
    bn_clean(tmp[i]);

  return result;
}

BigNum bn_fact_karatsuba(uint64_t a) {
  BigNum result = bn_with_capacity(1);
  result.components[0] = 1;

  if (!a)
    return result;

  for (uint64_t i = 1; i <= a; i++) {
    BigNum b = bn_with_capacity(1);
    b.components[0] = i;
    BigNum tmp = bn_mul_karatsuba(&result, &b);
    bn_clean(result);
    bn_clean(b);
    result = tmp;
  }

  return result;
}

BigNum bn_mul(BigNum a, BigNum b) {
  uint32_t a_used = bn_used_capacity(a);
  uint32_t b_used = bn_used_capacity(b);
  BigNum result = bn_with_capacity(a_used + b_used);

  for (int i = 0; i < a_used; i++)
    for (int j = 0; j < b_used; j++) {
      uint64_t product, carry;
      int ij = i + j;

      asm("mulq %%rbx;"
          : "=a"(product), "=d"(carry)
          : "a"(a.components[i]), "b"(b.components[j]));

      if ((result.components[ij] += product) < product)
        carry++;

      int k = 1;
      while (carry) {
        result.components[ij + k] += carry;
        carry = result.components[ij + k] < carry;
        k++;
      }
    }

  return result;
}

BigNum bn_fact(uint64_t a) {
  BigNum result = bn_with_capacity(1);
  result.components[0] = 1;

  if (!a)
    return result;

  for (uint64_t i = 1; i <= a; i++) {
    BigNum b = {1, &i};
    BigNum tmp = bn_mul(result, b);
    bn_clean(result);
    result = tmp;
  }

  return result;
}

BigNum bn_parse_hex(char *s) {
  if (!s)
    return bn_with_capacity(0);
  for (char *ss = s; *ss; ss++)
    if (!isxdigit(*ss))
      return bn_with_capacity(0);
  size_t len = strlen(s);
  if (!len)
    return bn_with_capacity(0);
  uint32_t a = ceil((double)len / 16);
  BigNum bn = bn_with_capacity(a);

  size_t partial = len % 16;

  if (partial) {
    char buf[16] = {0};
    memcpy(buf, s, partial);
    bn.components[bn.capacity - 1] = strtoul(buf, NULL, 16);
  }

  for (int i = partial ? 1 : 0; i < a; i++) {
    char buf[17] = {0};
    memcpy(buf, s + partial + (i - (partial ? 1 : 0)) * 16, 16);
    bn.components[bn.capacity - i - 1] = strtoul(buf, NULL, 16);
  }

  return bn;
}
