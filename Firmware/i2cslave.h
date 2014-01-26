/*
 * i2cslave.h
 *
 * Created: 1/18/2014 9:30:11 PM
 *  Author: Paul
 */ 


#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

volatile uint8_t buffer_address;
volatile uint8_t txbuffer[0xFF];
volatile uint8_t rxbuffer[0xFF];

void TWI_init(void);
void TWI_stop(void);

ISR(TWI_vect);

#endif /* I2CSLAVE_H_ */