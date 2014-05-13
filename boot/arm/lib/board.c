#include<config.h>
#define DEBUG

extern int _start;
extern int exception_table_end;
extern int exception_table;


void remap(void)
{
	int i = 0;
	int len = 1024;
	int *ptr = (int *)0x0;
	int *table = (int *)&exception_table;

	for(i = 0; &table[i] < &exception_table_end; i++ )
		ptr[i] = table[i];
#if 0
	check_remap();
#endif
}
int start_armboot(void)
{
	int status = 0;
	int  i = 0;
	serial_init();
	timer_init();
	remap();

	serial_puts("sys remap over\n");
    timer_irq_enable();
#if 1
	__raw_writel(0xffff, CFG_TIMERBASE + REG_TIMER_RELOAD);
#endif



	while(++i)
	{

		printf("while @ Start_armboot %x\n",i);
#if 1
		__udelay(1000);
#endif

#if 0
		__raw_writel(0x0, CFG_TIMERBASE + REG_TIMER_RELOAD);
#endif

	}

	return 0;
}

void success_fix(void)
{
	serial_puts("Suceess @ fix #\n");
	return;
}

void do_IRQ(void)
{
	__raw_writel(0xff, CFG_TIMERBASE + REG_TIMER_INTCLR);
	serial_puts("Suceess @ do_IRQ\n");
	return;
}
void enter_IRQ(void)
{
	int status = 0;
	__raw_writel(__raw_readl(CONFIG_IRQ_BASE + REG_IRQ_ENABLE),CONFIG_IRQ_BASE + 0x14);

	status = __raw_readl(CONFIG_IRQ_BASE + REG_IRQ_ENABLE);

	serial_puts("enter_IRQ\n");

}
void exit_IRQ(void)
{

	serial_puts("exit_IRQ\n");
	__raw_writel(1 << 3,CONFIG_IRQ_BASE + REG_IRQ_ENABLE);

}
