//library for phase 1 of blaster project
#include "i2c_motor.h"

uint8_t motor_init()
{
	if(!bcm2835_init())
	{
		printf("bcm2835 initialization failed!\n");
		return 1;
	}

	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	bcm2835_i2c_setSlaveAddress(0x00); //default to universal
	return 0;
}

void set_slave(uint8_t address)
{
	bcm2835_i2c_setSlaveAddress(address);
}

void motor_end()
{
	bcm2835_i2c_end();
	bcm2835_close();
}
	
uint8_t send_values(int8_t data)
{
	char wbuf[1];
	wbuf[0] = (data * SPEED_MAX) / 100;
	//wbuf[0] = convert_To_Pulse(thetaX);
	//wbuf[1] = convert_To_Pulse(thetaY);
	int8_t returnCondition = bcm2835_i2c_write(wbuf, 1); 
	return returnCondition;
}
