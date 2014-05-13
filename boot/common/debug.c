#include<config.h>
int irq_regs_show(void)
{
	int status = 0;
	int i = 0;
	do{
		status = __raw_readl(CONFIG_IRQ_BASE + i);
		printf("irq reg @ 0x%x = 0x%x\n", i, status);

		i += 4;
	}while(i <= 0x20);
}

void panic(int reg)
{
	printf("Arm panic @ 0x%x\n",reg);
}
void debug(int tag)
{
	printf("Arm debug @ 0x%x\n",tag);
}
