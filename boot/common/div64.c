int __div64_32(long *n, int base)
{
	long rem = *n;
	long b = base;
	long res, d = 1;
	int high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (long) high << 32;
		rem -= (long) (high*base) << 32;
	}

	while ((long)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}
