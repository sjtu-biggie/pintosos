#include <stdint.h>
#include <stdbool.h>
#include <debug.h>
#include "threads/fpoint.h"

#define FIXED_POINT_P 17
#define FIXED_POINT_Q (32 - FIXED_POINT_P - 1)

static bool is_overflow(int64_t value);
static int64_t f_value();

// return a * 2 ^ b
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


static int64_t f_value(){
    return pow2(1, FIXED_POINT_Q);
}

static bool is_overflow(int64_t value){
    return value > INT32_MAX || value < INT32_MIN;
}

fixed_point add(fixed_point a, fixed_point b){
    int64_t result = (int64_t)a + b;
    if(is_overflow(result)){
        //printf("overflow operation %d + %d = %lld\n", a, b, result);
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point add_constant(fixed_point a, int b){
    int64_t result =  a + (int64_t)b * f_value() ;
    if(is_overflow(result)){
        //printf("overflow operation %d + constant %d = %lld\n", a, b, result);
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
    int64_t result = (int64_t) a * b / f_value();
    if(is_overflow(result)){
        //printf("overflow operation %d * %d = %lld\n", a, b, result);
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point times_constant(fixed_point a, int b){
    int64_t result =  (int64_t)a * b;
    if(is_overflow(result)){
        //printf("overflow operation %d * constant %d = %lld\n", a, b, result);
        ASSERT(false);
    } 
    return result;
}

fixed_point div(fixed_point a, fixed_point b){
    ASSERT(b!=0)
    int64_t result = (int64_t) a * f_value() / b;
    if(is_overflow(result)){
        //printf("overflow operation %d / %d = %lld\n", a, b, result);
        ASSERT(false);
    } 
    return (fixed_point)result;
}

fixed_point div_constant(fixed_point a, int b){
    int64_t result =  (int64_t)a / b;
    if(is_overflow(result)){
        ASSERT(false);
        //printf("overflow operation %d / constant %d = %lld\n", a, b, result);
    } 
    return (fixed_point)result;
}

fixed_point to_fixed_point(int value){
    return add_constant(0, value);
}


int to_integer(fixed_point value){
    return (int64_t)value / f_value(); 
}

int to_integer_nearest(fixed_point value){
    // //printf("%lld\n",pow2(1, FIXED_POINT_Q) / 2 );
    if(value >= 0){
        return (value + f_value() / 2) / f_value(); 
    }else{
        return (value - f_value() / 2) / f_value(); 
    }
}
