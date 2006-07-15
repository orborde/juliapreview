#include <math.h>
#include "complex.h"

// square magnitude
double complex_sqmag ( const complex c )
{
  return (c.r * c.r) + (c.i * c.i);
}

// multiply two complex numbers
complex complex_mult (const complex a, const complex b)
{
  /*
    a + bi, c + di
    ac +adi + bic +bdi^2
  */
  complex ret;
  ret.r = (a.r * b.r - a.i * b.i);
  ret.i = (a.r * b.i + a.i * b.r);
  return ret;
}

// add two complex numbers
complex complex_add (const complex a, const complex b)
{
  complex ret = {a.r + b.r, a.i + b.i};
  return ret;
}
