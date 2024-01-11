/*
 * CarCar.c
 *
 * Created: 12/30/2023 12:49:24 AM
 *  Author: Asus
 */ 

#include "BasicFunction.h"
#include "Modules.h"


//on board LED PA1
int main(void)
{
	
	systemTimeInit();
	gpioInit(portA, 0, input, pullup);
	gpioInit(portA, 1, input, pullup);
	gpioInit(portA, 2, output, 0);
	gpioInit(portA, 3, output, 0);
	gpioInit(portA, 4, output, 0);
	/*
	uint8_t sport[9] = {portA, portA, portB, portB, portB, portD, portD, portD, portD};		//abcdefg12
	uint8_t spin[9]  = {  0  ,   2  ,   2  ,   3  ,   0  ,   1  ,   6  ,   7  ,   0  };
	ssegInit(sport, spin, 100);
	ssegEnable(1);
	*/
	float num1 = 1.234;
	float buff1 = 0;
	float num2 = 100.6;
	float buff2 = 0;
	
    while(1)
    {
		if(!gpioRead(portA, 0)){
			gpioWrite(portA, 2, 1);
			EEWrite(21, 4, (uint8_t*)(&num1));
			EEWrite(25, 4, (uint8_t*)(&num2 ));
			delay(1);
			gpioWrite(portA, 2, 0);
		}else if(!gpioRead(portA, 1)){
			gpioWrite(portA, 2, 1);
			EERead(21, 4, (uint8_t*)(&buff1));
			EERead(25, 4, (uint8_t*)(&buff2));
			delay(1);
			gpioWrite(portA, 2, 0);
		}
		
		if((buff2 - buff1) == (num2 - num1))
			gpioWrite(portA, 3, 1);
		else
			gpioWrite(portA, 3, 0);
		
		
    }
}