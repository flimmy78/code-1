#ifndef __IO_UGLY__
#define __IO_UGLY__

#define __raw_writel(v, a)   (*(volatile unsigned int *)(a) = (v))
#define __raw_readl(a)    (*(volatile unsigned int *)(a))

#endif
