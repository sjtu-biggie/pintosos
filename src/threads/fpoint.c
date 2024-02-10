#include <stdint.h>
#include <stdbool.h>
#include <debug.h>
#include "threads/fpoint.h"

#define FIXED_POINT_P 17
#define FIXED_POINT_Q (32 - FIXED_POINT_P)

static bool is_overflow(int64_t value);

static inline int64_t pow2(int64_t a, int b){
  if(b > 0){
    while(b-- > 0){
      a *= 2;
    }
  }else if(b < 0){
    while(b++ < 0){
      a /= 2;
    }
  }

  return a;
}



static bool is_overflow(int64_t value){
    return value <= INT32_MAX && value >= INT32_MIN;
}

fixed_point add(fixed_point a, fixed_point b){
    int64_t result = (int64_t)a + b;
    if(is_overflow(result)){
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point add_constant(fixed_point a, int b){
    int64_t result =  a + pow2((int64_t)b, FIXED_POINT_Q);
    if(is_overflow(result)){
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point sub(fixed_point a, fixed_point b){
    if(b != INT32_MAX){
        return add(a, -b);
    }else{
        //TODO: finish logic
        ASSERT(false);
    }
}

fixed_point sub_constant(fixed_point a, int b){
    if(b != INT32_MAX){
        return add_constant(a, -b);
    }else{
        //TODO: finish logic
        ASSERT(false);
    }
}

fixed_point times(fixed_point a, fixed_point b){
    int64_t result = pow2((int64_t) a * b, -FIXED_POINT_Q);
    if(is_overflow(result)){
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point times_constant(fixed_point a, int b){
    int64_t result =  (int64_t)a * b;
    if(is_overflow(result)){
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point div(fixed_point a, fixed_point b){
    int64_t result = pow2((int64_t) a, FIXED_POINT_Q) / b;
    if(is_overflow(result)){
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point div_constant(fixed_point a, int b){
    int64_t result =  (int64_t)a / b;
    if(is_overflow(result)){
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point to_fixed_point(int value){
    return add_constant(0, value);
}


int to_integer(fixed_point value){
    return pow2((int64_t)value, -FIXED_POINT_Q); 
}
