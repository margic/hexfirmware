/*
 * Firmware.c
 *
 * Created: 1/17/2014 10:21:57 PM
 *  Author: Paul
 */ 


#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2cslave.h"

void PWM_init(void);
void TWI_init(void);

int main(void)
{
	PWM_init();
	TWI_init();
	// allow interrupts
	sei();	
	while(1)
    {
		//loop
    }
}

void PWM_init(void){
	// initialize pwm reader on port b
	// set pin 0 or port b as input pin
	DDRB |= (1<<PINB0);	
	// enable pin change interrupt PCINT0-7
	// PCICR |= (1<<PCIE0); // replace with input capture
	// set pin 0 individually
	// PCMSK0 |= (1<<PCINT0);
	
	// enable input noise cancelling filter
	TCCR1B |= (1<<ICNC1); // adds 4 clock cycle delay to sample over 4 additional cycles
	// enable input capture on leading edge of ICP1
	TCCR1B |= (1<<ICES1);	
	// set timer prescaler to 1 for 2ms pulse counter at 8mhz
	TCCR1B |= (1<<CS10);
	// enable interrupt
	TIMSK1 |= (1<<ICIE1);
	
}

void TWI_init(void)
{
	//set TWI address
	TWAR = (0x01<<1); //using address 0x01
	//enable TWI
	// set the TWCR to enable address matching and enable TWI, clear TWINT, enable TWI interrupt
	TWCR = (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWEN);
}

void TWI_stop(void){
	// clear acknowledge and enable bits
	TWCR &= ~( (1<<TWEA) | (1<<TWEN) );
}

ISR(TIMER1_CAPT_vect){
	// Input capture interrupt handler
	// first time is triggered in leading edge
	if(PORTB & (0b00000001)){ // pin high 
		// clear counter and set to trailing edge
		TCNT1 = 0;
		TCCR1B &= ~(1<<ICES1);
	}else{
		// get counter from ICR1
		// reset to look for leading edge
		uint_fast16_t curVal = ICR1;
		TCCR1B |= (1<<ICES1);  
	}
}

ISR(TWI_vect){
	
	// temporary stores the received data
	uint8_t data;
	
	// own address has been acknowledged
	if( (TWSR & 0xF8) == TW_SR_SLA_ACK ){
		buffer_address = 0xFF;
		// clear TWI interrupt flag, prepare to receive next byte and acknowledge
		TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
	}
	else if( (TWSR & 0xF8) == TW_SR_DATA_ACK ){ // data has been received in slave receiver mode
		
		// save the received byte inside data
		data = TWDR;
		
		// check wether an address has already been transmitted or not
		if(buffer_address == 0xFF){
			
			buffer_address = data;
			
			// clear TWI interrupt flag, prepare to receive next byte and acknowledge
			TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
		}
		else{ // if a databyte has already been received
			
			// store the data at the current address
			rxbuffer[buffer_address] = data;
			
			// increment the buffer address
			buffer_address++;
			
			// if there is still enough space inside the buffer
			if(buffer_address < 0xFF){
				// clear TWI interrupt flag, prepare to receive next byte and acknowledge
				TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
			}
			else{
				// clear TWI interrupt flag, prepare to receive last byte and don't acknowledge
				TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN);
			}
		}
	}
	else if( (TWSR & 0xF8) == TW_ST_DATA_ACK ){ // device has been addressed to be a transmitter
		
		// copy data from TWDR to the temporary memory
		data = TWDR;
		
		// if no buffer read address has been sent yet
		if( buffer_address == 0xFF ){
			buffer_address = data;
		}
		
		// copy the specified buffer address into the TWDR register for transmission
		TWDR = txbuffer[buffer_address];
		// increment buffer read address
		buffer_address++;
		
		// if there is another buffer address that can be sent
		if(buffer_address < 0xFF){
			// clear TWI interrupt flag, prepare to send next byte and receive acknowledge
			TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
		}
		else{
			// clear TWI interrupt flag, prepare to send last byte and receive not acknowledge
			TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN);
		}
		
	}
	else{
		// if none of the above apply prepare TWI to be addressed again
		TWCR |= (1<<TWIE) | (1<<TWEA) | (1<<TWEN);
	}
}