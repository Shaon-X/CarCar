struct sseg_t{
	
	uint8_t port[9];
	uint8_t pin[9];
	uint8_t val[2];
	uint8_t flag;		//bit0: enable,   bit1: digit
	float t;
	float interval;
};

struct sseg_t sseg;

void ssegInit(uint8_t* port, uint8_t* pin, float freq){
	
	for(uint8_t i=0; i<9; i++){
		sseg.port[i] = *(port+i);
		sseg.pin[i] = *(pin+i);
		gpioInit(*(port+i), *(pin+i), output, 0);
	}
	sseg.val[0] = sseg.val[1] = 0;
	sseg.flag = 0;
	sseg.t = time();
	sseg.interval = 1.0/freq;
}

void ssegEnable(uint8_t en){
	if(en)
		sseg.flag |= 0b00000001;
	else
		sseg.flag &= 0b11111110;
		
}

void ssegSet(uint8_t digit, uint8_t val){
	sseg.val[digit-1] = val;
}

/*
Function:	ssegWrite
Arguments:	digit		1(first digit) or 2(second digit)
			val			0 to 9 to be shown
*/
void ssegWrite(uint8_t digit, uint8_t val){
	uint8_t buff;
	switch(val){
		case 0:
			buff = 0b11111100;
			break;
		case 1:
			buff = 0b01100000;
			break;
		case 2:
			buff = 0b11011010;
			break;
		case 3:
			buff = 0b11110010;
			break;
		case 4:
			buff = 0b01100110;
			break;
		case 5:
			buff = 0b10110110;
			break;
		case 6:
			buff = 0b10111110;
			break;
		case 7:
			buff = 0b11100000;
			break;
		case 8:
			buff = 0b11111110;
			break;
		case 9: 
			buff = 0b11110110;
			break;
		default:
			buff = 0b00000010;
			break;
	}
		
		buff |= digit&0b00000001;
		
		gpioWrite(sseg.port[7], sseg.pin[7], 1);
		gpioWrite(sseg.port[8], sseg.pin[8], 1);
		for(uint8_t i=0; i<8; i++)
			gpioWrite(sseg.port[i], sseg.pin[i], (buff>>(7-i))&0b00000001);
		gpioWrite(sseg.port[8], sseg.pin[8], (~buff&0b00000001));
		
}


void ssegUpdate(){
	if(!(sseg.flag&0b00000001)){
		gpioWrite(sseg.port[7], sseg.pin[7], 1);
		gpioWrite(sseg.port[8], sseg.pin[8], 1);
	//}else if(time() - sseg.t > sseg.interval){
	}else{
		if(sseg.flag&0b00000010)
			ssegWrite(2, sseg.val[1]);
		else
			ssegWrite(1, sseg.val[0]);
		sseg.flag ^= 0b00000010;
		//sseg.t = time();
	}
	
}
/*
Function:	TCSInit
Return:		1(error during TWI) or 0(no error)
*/
uint8_t TCSInit(){
	
	uint8_t err=0;
		
	uint8_t twiBuff[2]={0b10000001, 0xD5};	//ATIME register
	err = twiWrite(0x29, 2, twiBuff);
	twiBuff[0] = 0b10001111;				//CONTROL register
	twiBuff[1] = 0;
	err = twiWrite(0x29, 2, twiBuff);
	return err;
	
}

/*
Function:	TCSEnable
Return:		1(error during TWI) or 0(no error)
*/
uint8_t TCSEnable(){
	
	uint8_t twiBuff[2]={0b10000000, 0b00000011};	//ENABLE register
	delay(0.024);
	return twiWrite(0x29, 2, twiBuff);
	
}

/*
Function:	TCSDisable
Return:		1(error during TWI) or 0(no error)
*/
uint8_t TCSDisable(){
	
	uint8_t twiBuff[2]={0b10000000, 0b00000001};	//ENABLE register
	return twiWrite(0x29, 2, twiBuff);
	
}

/*
Function:	TCSSleep
Return:		1(error during TWI) or 0(no error)
*/
uint8_t TCSSleep(){
	
	uint8_t twiBuff[2]={0b10000000, 0b00000000};	//ENABLE register
	return twiWrite(0x29, 2, twiBuff);
	
}

/*
Function:	TCSRead
Arguments:	*data		pointer to data location to store RGB values
Return:		1(error during TWI) or 0(no error)
*/
uint8_t TCSRead(uint16_t* data){
	uint8_t err = 0;
	union{
		uint8_t twi8[6];
		uint16_t twi16[3];
	}twiu;
	err = twiCombine(0x29, 0b10110110, 6, twiu.twi8);		//read 6 bytes 
	if(!err){
		for(uint8_t i=0; i<3; i++)
			*(data+i) = twiu.twi16[i];
	}
	return err;
	
}


uint8_t TCSRed(uint16_t *data){
	
	if((*data) > *(data+1)+10 && (*data) > *(data+2)+10)
		return 1;
	else
		return 0;
	
}

uint8_t MPUInit(){
	
	delay(0.03);
	
	uint8_t twiBuff[2]={107, 0x04};	//PWR_MGMT_1 register
		
	return twiWrite(0b1101000, 2, twiBuff);
	
}

void MPURead(float* data){
	uint8_t buf[2];
	union{
		uint8_t twi8[2];
		int16_t twi16;
	}twiu;
	if(!twiCombine(0b1101000, 71, 2, buf)){
		
		twiu.twi8[0] = buf[1];
		twiu.twi8[1] = buf[0];
		twiu.twi16 -= 85;
		*data += ((float)twiu.twi16 / 131.0) * 0.016384;
		
	}
	
}

void motor(int8_t motorL, int8_t motorR){
	
	if(motorL >= 0){
		gpioWrite(IN1, 1);
		gpioWrite(IN2, 0);
	}else{
		gpioWrite(IN1, 0);
		gpioWrite(IN2, 1);
		motorL = -motorL;
	}
	if(motorR >= 0){
		gpioWrite(IN3, 1);
		gpioWrite(IN4, 0);
	}else{
		gpioWrite(IN3, 0);
		gpioWrite(IN4, 1);
		motorR = -motorR;
	}
	
	pwmSet(motorL, motorR);
	
	
}
