#ifndef __COMPLEX_H
#define __COMPLEX_H

typedef struct _complex
{
  double r,i;
}
complex;

// square magnitude
double complex_sqmag ( const complex c );

// multiply two complex numbers
complex complex_mult (const complex a, const complex b);

// add two complex numbers
complex complex_add (const complex a, const complex b);

#endif
