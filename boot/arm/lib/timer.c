/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#define TIMER_LOAD_VAL 0xffffffff

unsigned long get_timer_masked(void);
void reset_timer_masked(void);

/*
 * reset			timestamp
 * ------------------------------------->time goes by
 *		^			^
 *	      lastdec			now
 */
static unsigned long timestamp; /* how long since last timer reset */
static unsigned long lastdec;


int timer_init(void)
{
	/* enable the reference clk. but we did not find the reset switch */
	__raw_writel(__raw_readl(REG_BASE_SCTL + REG_SC_CTRL)
			| (1<<16) | (1<<18) | (1<<20),
			REG_BASE_SCTL + REG_SC_CTRL);
	/*
	 * Under uboot, 0xffffffff is set to load register,
	 * timer_clk = BUSCLK/2/256.
	 * e.g. BUSCLK = 50M, it will roll back after 0xffffffff/timer_clk
	 * = 43980s = 12hours
	 */
	__raw_writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	__raw_writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);

	/* 32 bit, periodic,  256 divider. */
	__raw_writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER;  /* capure current decrementer value time */
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

unsigned long get_timer(unsigned long base)
{
	return get_timer_masked() - base;
}

unsigned long get_timer_masked(void)
{
	unsigned long now = READ_TIMER;		/* current tick value */

	if (lastdec >= now) {
		/* normal mode, not roll back */
		timestamp += lastdec - now;
	} else {
		/* rollback */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

void set_timer(unsigned long t)
{
	timestamp = t;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
unsigned long get_tbclk(void)
{
	unsigned long tbclk;

	tbclk = CONFIG_SYS_HZ;

	return tbclk;
}

/* delay x useconds AND perserve advance timstamp value */
void __udelay(unsigned long usec)
{
	unsigned long tmo, tmp;

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CONFIG_SYS_HZ;
		/* finish normalize. */
		tmo /= 1000;
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000 * 1000);
	}

	tmp = get_timer(0);	/* get current timestamp */
	/* if setting this fordward will roll time stamp */
	if ((tmo + tmp + 1) < tmp)
		/* reset "advancing" timestamp to 0, set lastdec value */
		reset_timer_masked();
	else
		/* else, set advancing stamp wake up time */
		tmo += tmp;

	while (get_timer_masked() < tmo)
		;/* loop till event */
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
{
	unsigned long tmo;
	unsigned long endtime;
	signed long diff;

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
		/* finish normalize. */
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000 * 1000);
	}

	endtime = get_timer_masked() + tmo;

	do {
		unsigned long now = get_timer_masked();
		diff = endtime - now;
	} while (diff >= 0);
}
