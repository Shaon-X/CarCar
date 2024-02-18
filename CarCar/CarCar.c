/*
 * CarCar.c
 *
 * Created: 12/30/2023 12:49:24 AM
 *  Author: Asus
 */ 

#define portA		0x19
#define portB		0x16
#define portC		0x13
#define portD		0x10

#define IRl		portB, 3
#define IRr		portC, 6

#define BT1		portD, 3
#define BT2		portD, 6

#define IN1		portB, 4
#define IN2		portB, 5
#define IN3		portB, 6
#define IN4		portB, 7

#define TCSLED	portB, 2

#define Kp		1.5;
#define Mp		15


#include "BasicFunction.h"
#include "Modules.h"

float yaw=0, yawDat, yawOffset=0;
uint8_t TCSCount = 0, MPUCount = 0;
uint16_t TCSData[3] = {0, 0, 0};
uint8_t state = 0;
float timeElapsed = 0;
uint8_t buff, buff2;
uint8_t area[6] = {0, 0, 0, 0, 0, 0};

void Init();
void idle();
void show();
void run();

/*
Timer1 Interrupt Vector

ISR(TIMER1_OVF_vect){
	TCNT1 = 65536 - 1250;	//reset timer1 count
	sysTick++;				//increase system tick
	if(state == 2){
		MPURead(&yaw);
		TCSCount++;
		if(TCSCount >= 20){
			TCSRead(&TCSData);
			TCSCount = 0;
		}
	}
}
*/

/*
Timer2 Interrupt Vector
*/
ISR(TIMER2_OVF_vect){
	sysTick++;				//increase system tick
	ssegUpdate();
	if(state == 2){
		TCSCount++;
		
		MPUCount++;
		if(MPUCount >=2){
			MPURead(&yawDat);
			MPUCount = 0;
		}

		if(TCSCount >= 20){
			TCSRead(TCSData);
			TCSCount = 0;
		}
		
	}
}


int main(void)
{
	
	Init();
	
//	uint8_t str = '\n';

    while(1)
    {

/*
		state = 2;
		uartTransmitInt((int)yaw);
		uartTransmit(&str, 1, 0.2);
		delay(0.5);
*/
	


		if(!state){
			
			idle();
			
		}else if(state == 1){
			
			show();
			
		}else if(state == 2){	
			
			run();
			
		}	
		

		
    }
	
}

void Init(){
	systemTimeInit();
	
	//Basic Functions
	gpioInit(IRl, input, pullup);		
	gpioInit(IRr, input, pullup);	
	gpioInit(BT1, input, pullup);
	gpioInit(BT2, input, pullup);
	gpioInit(TCSLED, output, 0);
	gpioInit(IN1, output, 0);
	gpioInit(IN2, output, 0);
	gpioInit(IN3, output, 0);
	gpioInit(IN4, output, 0);
	
	pwmInit();
	motor(0, 0);
	twiInit();
	uartInit();
	
	//Modules
	uint8_t sport[9] = {portA, portA, portA, portA, portA, portA, portA, portA, portC};		//abcdefg12
	uint8_t spin[9]  = {  0  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  ,   7  ,  7   };
	ssegInit(sport, spin, 100);
	TCSInit();
	TCSEnable();
	MPUInit();
	
	

}

void idle(){
	
	motor(0, 0);
	gpioWrite(TCSLED, 0);
	if(gpioReadDebounce(0)){
		state = 2;
		yawOffset = yawDat;
		buff = 0;
		buff2 = 0;
		for(uint8_t i=0; i<6; i++)
			area[i] = 0;
		gpioWrite(TCSLED, 1);
		timeElapsed = time();
	}else if(gpioReadDebounce(1)){
		state = 1;
		timeElapsed = time();
	}
	
}

void show(){
	
	ssegEnable(1);
	if(!area[0]){
		ssegSet(1, 10);
		ssegSet(2, 10);
		while(time() - timeElapsed < 1 && !gpioReadDebounce(1));
	}
	buff = 0;
	while(area[buff]){
		if(buff == 6)
			break;
		ssegSet(1, buff+1);
		ssegSet(2, area[buff]);
		if(gpioReadDebounce(1))
		break;
		
		if(time() - timeElapsed >= 1){
			timeElapsed = time();
			buff++;
		}
	}
	
	state = 0;
	ssegEnable(0);
	
}

void run(){
	
	int8_t motorL = 40, motorR = 40;
	int32_t Pout;
	float target = 0;
	static float angleOffset = 0;
	static uint8_t flag = 0;	//4- box 3 turn   3-RGB already sense     2-RGB not pass area     1-IRr     0- IRl   
	static float IRTime = 0;
	static float boxTime = 0;
	static float turnTime = 0;
	
	yaw = yawDat - yawOffset;
	
	if(gpioReadDebounce(0) || buff == 7){
		state = 0;
		flag = 0;
		angleOffset = 0;
	}
		
	if(time() - IRTime > 1)
		flag &= 0b11111100;
	
	
	if(!(flag & 0b00001100) && buff > 0 && TCSRed(TCSData)){
		area[buff2++] = buff;
		flag |= 0b00001000;
	}
	
	
	if(flag & 0b00010000){
		
		angleOffset = yaw;
		if(time() - turnTime < 0.7){
			motorL = motorR = -40;
		}else if(flag & 0b01000000){
			if(time() - turnTime < 0.5){
				motorL = motorR = 0;
			}else if(time() - turnTime < 1){
				motorL = motorR = -40;
			}else{
				flag &= 0b11101111;
				angleOffset = 90;
			}
		}else{
			motorL = -40;
			if(90.0 - yaw < 3 && 90.0 - yaw > -3){
				flag |= 0b01000000;
				turnTime = time();
				motorR = -40;
			}
		}
		
	}else if(flag & 0b00100000){
		
		angleOffset = yaw;
		if(time() - turnTime < 0.7){
			motorL = motorR = -40;
		}else if(flag & 0b10000000){
			if(time() - turnTime < 0.5){
				motorL = motorR = 0;
			}else if(time() - turnTime < 1){
				motorL = motorR = -40;
			}else{
				flag &= 0b11101111;
				angleOffset = 90;
			}
		}else{
			motorL = -40;
			if(180.0 - yaw < 3 && 180.0 - yaw > -3){
				flag |= 0b10000000;
				turnTime = time();
				motorR = -40;
			}
		}
		
	}else if(time() - boxTime > 0.5){
		ssegEnable(0);
		if(flag&0b00000100){
			buff++;
			flag &= 0b11110000;
		}
		
		if(gpioRead(IRl)){
			if(flag&0b00000010)	{
				flag &= 0b11111100;
				if(buff == 3 && !(flag & 0b01000000)){
					flag |= 0b00010000;
					turnTime = time();
				}else if(buff == 4 && !(flag & 0b10000000)){
					flag |= 0b00100000;
					turnTime = time();	
				}else{
					boxTime = time();
					flag |= 0b00000100;
				}
			}else{
				IRTime = time();
				flag |= 0b00000001;
			}
		}
		
		if(gpioRead(IRr)){
			if(flag&0b00000001)	{
				flag &= 0b11111100;
				if(buff == 3 && !(flag & 0b01000000)){
					flag |= 0b00010000;
					turnTime = time();
				}else if(buff == 4 && !(flag & 0b10000000)){
					flag |= 0b00100000;
					turnTime = time();
				}else{
					boxTime = time();
					flag |= 0b00000100;
				}
			}else{
				IRTime = time();
				flag |= 0b00000010;
			}
		}
		
	}else{
		ssegEnable(1);
		ssegSet(1, buff+1);
		ssegSet(2, 10);
		
		flag &= 0b11111100;
	}
	
	
	
	if(flag & 0b00000001)
		target = angleOffset-20;
	else if(flag & 0b00000010)
		target = angleOffset+20;
	else
		target = angleOffset;
	
	Pout = (target-yaw) * Kp;
	
	if(Pout > Mp)
		Pout = Mp;
	else if(Pout < -Mp)
		Pout = -Mp;
	
	motorL -= Pout;
	motorR += Pout;
	
	motor(motorL, motorR);
	
}