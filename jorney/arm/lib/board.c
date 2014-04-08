
int start_armboot(void)
{
	int i = 0;
	serial_init();
	timer_init();
	enable_irq();

	while(++i)
	{
		__udelay(1000000);
		serial_puts("You did it\n");
	}
	return 0;
}
