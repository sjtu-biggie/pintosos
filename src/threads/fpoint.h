#ifndef THREADS_FPOINT_H
#define THREADS_FPOINT_H

#include <debug.h>

typedef int fixed_point;

fixed_point add(fixed_point a, fixed_point b);
fixed_point add_constant(fixed_point a, int b);
fixed_point sub(fixed_point a, fixed_point b);
fixed_point sub_constant(fixed_point a, int b);
fixed_point times(fixed_point a, fixed_point b);
fixed_point times_constant(fixed_point a, int b);
fixed_point div(fixed_point a, fixed_point b);
fixed_point div_constant(fixed_point a, int b);
fixed_point to_fixed_point(int value);

int to_integer(fixed_point value);
int to_integer_nearest(fixed_point value);

#endif