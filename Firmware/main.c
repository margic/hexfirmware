// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* - File              : main.c
* - Compiler          : IAR EWAAVR 3.20
*
* - Support mail      : avr@atmel.com
*
* - Supported devices : All devices with an Input Capture Unit can be used.
*                       The example is written for ATmega64
*                       The demonstration is tuned for a 3.6864MHz clock,
*                       but clock speed is not critical to correct operation.
*
* - AppNote           : AVR135 - Pulse-Width Demodulation
*
* - Description       : Example of how to use Pulse-Width Demodulation
*
* Originally authored by Bruce McKenny
* $Revision: 1.5 $
* $Date: Wednesday, November 02, 2005 13:20:02 UTC $
*****************************************************************************/

/*! \mainpage
* \section Intro Introduction
* This documents functions, variables, defines, enums and typedefs in the software
* for application note AVR135: Using Timer Capture to Measure PWM Duty Cycle\n
*
* \section Descr Description
* icp.c contains functions for use of the demodulator.\n
* main.c contains a simple program to demonstrate the demodulator operation. It
* generates a PWM signal on OC2, then samples the demodulator output using calls
* to icp_rx() and writes the values obtained to PORTC.\n
* Approximately twice per second, OCR2 is incremented such that the PWM duty cycle
* steps through the entire [0:256) range.\n
* The intended use is on the STK500/STK501, with a 10-wire jumper connecting the
* PORTC header with the LEDs header and a single-wire jumper connecting PB7 (OC2)
* with PD4 (ICP1).\n
* The expected result is that the LED display cycles repeatedly from 0x00 to 0xFF.\n
*
* \section CI Compilation Info
* This software was written for the IAR Embedded Workbench, 3.20C, but can also be
* built using newer versions of IAR, and also GCC (see note below).\n
* To make project for IAR EWAVR:
* Add the .c files to project (main.c and icp.c). Use device --cpu=m64, small memory
* model, optimization low for debug target and high for release, output format:
* ubrof8 for Debug and intel_extended for Release. \n
*
* A makefile is included for use with GCC. Defines are included in code to make
* porting to GCC easy.\n
*
* \section DI Device Info
* This example is written for ATMega64, but all devices with an Input Capture Unit
* can be used.\n
* The demonstration is tuned for a 3.6864MHz clock, but clock speed is not critical
* to correct operation.\n
*/

#if __GCC__
#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>	/* sei */
#define	SEI()	sei()
#define	CLI()	cli()
#define	SLEEP()	__asm__ __volatile__ ("sleep \r\n" :: )
#else /* IAR */
#include <stdio.h>
#include <inavr.h>
#define ENABLE_BIT_DEFINITIONS
#include <iom64.h>
#define	SEI()	asm("SEI")
#define	CLI()	asm("CLI")
#define	SLEEP()	asm("SLEEP")
#endif

#include "icp.h"

/*
 * LED debug definitions (ATmega64).
 * The current detected PWD duty cycle is displayed on PC[7:0]
 */
#define	LED_DEBUG_DDR	DDRC
#define	LED_DEBUG_PORT	PORTC

/**
 * TIMER0_OVF()
 *
 * Overflow ISR for timer0.
 * Updates the PWM duty cycle (timer2).
 */
unsigned char hb_count;			/* counts [0:4] to make the test visible */
#if __GCC__
SIGNAL(SIG_OVERFLOW0)
#else
#pragma vector=TIMER0_OVF_vect
__interrupt void TIMER0_OVF(void)
#endif
{
	if (++hb_count >= 4)	/* apx 0.25 seconds */
	{
		/*
		 * Clear the post-scale counter
		 */
		hb_count = 0;

		/*
		 * Update the duty cycle (OCR2). The result is allowed
		 * to overflow back to 0 to restart the run.
		 */
		OCR2 += 1;
	}
	return;
}

/**
 * hb_init()
 *
 * Start up heartbeat timer.
 *
 * It updates the duty cycle for the PWM train being Demodulated.
 * Using the STK500's 3.6864MHz clock, and a /1024 prescale, the
 * (8-bit) timer overflows at about 14Hz. A counter [0:4) in the ISR
 * slows this down by 4, so the duty cycle changes apx. 3.5Hz.
 * This isn't very precise -- it just needs to be slow enough to
 * be visible on the LEDs.
 */
void
hb_init(void)
{
	/*
	 * The heartbeat timer (timer0) is used in free-running mode.
	 */
	ASSR = 0;	/* AS0 = 0 */;	/* disable asynchronous mode */
	while (ASSR) /*EMPTY*/;
	TCCR0 = (1 << CS02) | (1 << CS01) | (1 << CS00); /* prescale /1024 */
	TIMSK |= (1 << TOIE0);				/* enable overflow interrupt */

	return;
}

/**
 * pwm_init()
 *
 * Start up PWM (timer2) for demonstration purposes.
 * It drives a non-inverted PWM train on the OC2 pin, which will be
 * Demodulated by the ICP1 ISR (icp.c).
 * A patch wire should be used to connect OC2 (PB7) with ICP1 (PD4).
 */
void
pwm_init(void)
{
	/*
	 * Set up PWM using timer2
	 */
	DDRB |= (1 << PB7);		/* enable PWM (OC2) output pin */
	OCR2 = 0;				/* start at 0% */
	TCCR2 = (1 << WGM21) | (1 << WGM20) 	/* fast PWM */
		  |	(1 << COM21) | (0 << COM20)		/* non-inverted PWM */
		  |	(0 << CS22)  | (1 << CS21) | (1 << CS20)  /* prescale /64	*/
			;
	return;
}

/**
 * main()
 *
 * Demonstration main program.
 */
int
main(void)
{
	icp_sample_t sample;

	/*
	 * Init subsystems
	 */
	hb_init();						/* main.c	*/
	icp_init();						/* icp.c	*/
	pwm_init();						/* main.c	*/

	/*
	 * PORTC for LED debug
	 */
	LED_DEBUG_PORT = 0xFF;			/* initially off */
	LED_DEBUG_DDR = 0xFF;			/* all output */

	/*
	 * The ISRs do most of the work.
	 */
	SEI();							/* enable interrupts since init is done */

	MCUCR |= (1 << SE);				/* enable (idle mode) sleep */

	/*
	 * Loop forever
	 */
	for (;;)
	{
		/*
		 * Fetch the latest reading and display it on the LEDs.
		 */
		sample = icp_rx();

		LED_DEBUG_PORT = ~sample;

		/*
		 * Sleep until the next interrupt. This will wake up twice
		 * per PWM period, plus (apx.) 4 times per second for the
		 * heartbeat/update timer. This is more often than needed,
		 * but is certainly sufficient for this demonstration.
		 */
		SLEEP();
	}
	/*NOTREACHED*/
	return(0);
}
