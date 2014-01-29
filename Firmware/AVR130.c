//***************************************************************************
// A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
//
// Number               : AVR130
// File Name            : "avr130.c"
// Title                : Setup and Use The Timers
// Date                 : 00.09.01
// Version              : 1.0
// Target MCU           : Any AVR with Timer
//
// DESCRIPTION
// This Application note shows how to enable and use some features of the 
// AVR's on-board Timers. 
//
// This file contains the code of all the examples. To switch between the examples
// you will have to enable the call of the corresponding initialization routine
// (see the main program at the bottom) 
//
// This code contains markers (e.g. CL_1). These markers indicate to which step 
// of the check list the 1-2 code lines which follow the marker refer to. 
//
//**************************************************************************


#include <stdio.h>
#define __IAR_SYSTEMS_ASM__
#include "io8535.h"
#include "ina90.h"

// Initialization Routine Example 1
void init_Ex1(void)
{
// (CL_1)
  TCCR0 = (1<<CS02)|(1<<CS00);  // Timer Clock = system clock / 1024
// (CL_2.0)
  TIFR  = 1<<TOV0;		// Clear TOV0 / Clear pending interrupts
// (CL_2.1)
  TIMSK = 1<<TOIE0;             // Enable Timer 0 Overflow Interrupt
  DDRB  = 0xFF;                 // Set Port B as output
}

// Initialization Routine Example 2
void init_Ex2(void)
{
// (CL_1)
  TCCR1B = (1<<CS11)|(1<<CS10); // Timer Clock = system clock / 64
// (CL_2.0)
  TIFR   = 1<<ICF1;		// Clear ICF1 / Clear pending interrupts
// (CL_2.1)
  TIMSK  = 1<<TICIE1;           // Enable Timer 1 Capture Event Interrupt
  DDRB   = 0xFF;                // Set Port B as output
  DDRD   &= ~(1<<PD6);          // Set PD6 as input  
}

// Initialization Routine Example 3
void init_Ex3(void)
{
// (CL_1)
  // Clear Timer on compare match / Timer Clock = system clock / 1024
  TCCR2 = (1<<CTC2)|(1<<CS22)|(1<<CS21)|(1<<CS20); 
// (CL_2.0)
  TIFR  = OCF2;			// Clear OCF2 / Clear pending interrupts
// (CL_2.1)
  TIMSK = 1<<OCIE2;		// Enable Timer 2 Output Compare Match Event Interrupt
  OCR2  = 32;			// Set Output Compare Value to 32
  ASSR  = 1<<AS2;		// Enable asynchronous mode
  DDRB  = 0xFF;			// Set Port B as output
}

// Initialization Routine Example 4
void init_Ex4(void)
{
// (CL_2.1)
  // Enable non inverting 8Bit PWM; Timer Clock = system clock/1
  TCCR2 = (1<<COM21)+(1<<PWM2)|(1<<CS20); 
  DDRD  = (1 << PD7)|0x0F;      // PD7 (OC2) and PD0-PD3 as output 
  // Set Output Compare Value to 0xE0 => duty rate 1/8 to 7/8
  OCR2  = 0xE0;			 
}

// Example 1 - ISR Overflow Interrupt Timer 0 
// (CL_2.4)
void interrupt [TIMER0_OVF0_vect] ISR_TOV0 (void)
{
  PORTB = ~PORTB;		// Toggle pins on Port B
}

// Example 2 - ISR Input Capture Interrupt Timer 1 
// (CL_2.4)
void interrupt [TIMER1_CAPT1_vect] ISR_ICP1(void)
{ 
  // read high byte from Input Capture Register (read 16 bit value and shift it eight bits to the right)
  PORTB = ~( ICR1>>8);         	// invert Byte (see Note 1) and output high byte on Port B 
  TCNT1 = 0;                    // reset Timer Count Register
}

// Example 3 - ISR Output Compare Interrupt Timer 2
// (CL_2.4)
void interrupt [TIMER2_COMP_vect] ISR_OCIE2 (void)
{
  PORTB = ~PORTB;               // swap bits on Port B 
}


void main (void)
{
// Enable the next line for Example 1 disable for other Examples
  init_Ex1();
// Enable the next line for Example 2 disable for other Examples
//  init_Ex2();
// Enable the next line for Example 3 disable for other Examples
//  init_Ex3();
// Enable the next line for Example 4 disable for other Examples
//  init_Ex4();
// (CL_2.2)
  _SEI();                       // enable global interrupts

// endless loop
  for (;;){}
}



// Note 1: The inversion of the bits is necessary because of the way the LED's are 
// connected on the STK500 (Low Level = LED on / High Level = LED off)
