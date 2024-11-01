#ifndef MACRO_HEADER_INCLUDED
#define MACRO_HEADER_INCLUDED

#define LSHIFT(x, y) ((x) << (y))

#if ((((int32_t)-1) >> 1) == ((int32_t)-1))
#define RSHIFT(x, y) ((x) >> (y))
#else
#define RSHIFT(x, y) ((((uint64_t)(x) + 0x80000000UL) >> (y)) - (0x80000000UL >> (y)))
#endif

#define CONVERT_INT32_TO_UINT32(value) ((uint32_t)((LSHIFT((value), 1)) ^ (RSHIFT((value), 31))))
#define CONVERT_UINT32_TO_INT32(value) ((int32_t)(RSHIFT((value), 1) ^ (-(int32_t)((value) & 1))))

#define POLYNOMIAL_PREDICATOR_MAX_HISTORY   2

#endif
