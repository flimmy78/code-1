/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 * Based on former asm-ppc/div64.h and asm-m68knommu/div64.h
 *
 * The semantics of do_div() are:
 *
 * int do_div(long *n, int base)
 * {
 *	int remainder = *n % base;
 *	*n = *n / base;
 *	return remainder;
 * }
 *
 * NOTE: macro parameter n is evaluated multiple times,
 *       beware of side effects!
 */


extern int __div64_32(long *dividend, int divisor);


/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
# define do_div(n,base) ({				\
	int __base = (base);			\
	int __rem;					\
	(void)(((typeof((n)) *)0) == ((long *)0));	\
	if (((n) >> 32) == 0) {			\
		__rem = (int)(n) % __base;		\
		(n) = (int)(n) / __base;		\
	} else						\
		__rem = __div64_32(&(n), __base);	\
	__rem;						\
 })

