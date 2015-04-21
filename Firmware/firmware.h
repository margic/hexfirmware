/*
 * i2cslave.h
 *
 * Created: 1/18/2014 9:30:11 PM
 *  Author: Paul
 */ 


#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

#define PIEVENT PIND4
#define PLEXE PIND3
#define PLEXS0 PIND0
#define PLEXS1 PIND1
#define PLEXS2 PIND2

#define NUMCHANNELS 6

volatile uint8_t buffer_address;
volatile uint8_t txbuffer[0xFF];
volatile uint8_t rxbuffer[0xFF];

void switchMultiplex(void);
void TWI_init(void);
void TWI_stop(void);

ISR(TWI_vect);

#endif /* I2CSLAVE_H_ */