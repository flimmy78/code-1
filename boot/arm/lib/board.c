int start_armboot(void)
{
	serial_init();
	timer_init();
	while(1){
		__udelay(100000);
		serial_puts("Asshole , you did it!\n");
	}
	return 0;
}
