#include <bcm2835.h>
#include <stdio.h>
#define SPEED_MIN -127
#define SPEED_MAX 127



uint8_t motor_init();
void set_slave(uint8_t address);
void motor_end();
uint8_t send_values(int8_t data);
