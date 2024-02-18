
//TWI status register values
#define Start		0x08
#define RStart		0x10
#define AddW		0x18
#define DatW		0x28
#define AddR		0x40
#define DatR		0x50
#define DatRN		0x58

uint32_t sysTick;			//system tick

/*
Function:	systemTimeInit
Remark:		only called at the start
*/
void systemTimeInit(){
	/*	prescaler	= 32
		clk freq	= 1MHz / 8	= 31250
		period		= 32 microsec
		interrupt	= 0.008192 sec
	*/

	sysTick = 0;			//initialize system tick
	TCNT2 = 0;				//initialize timer2 count
	TIFR = 0b01000000;		//clear TOV2 bit
	TIMSK |= 0b01000000;	//enable enable timer2 overflow interrupt
	sei();					//enable global interrupt in status register
	
	TCCR2 = 0b00000011;		//normal mode, prescaller 32
}

/*
Function:	tick
*/
uint32_t tick(){
	return sysTick * (uint32_t)256 + (uint32_t)TCNT2;	//return number of ticks
}

/*
Function:	time
*/
float time(){
	return (float)tick()*0.000032;	//return elapsed time
}

/*
Function:	gpioInit
Arguments:	port		portX, X:A, B, C, D
			pin			0 - 7
			io			input, output
			arg			For input: pullup, !pullup 
						For output(initial value):1, 0
*/
void gpioInit(uint8_t port, uint8_t pin, uint8_t io, uint8_t arg){
	
	SFIOR &= ~4;															//enable pull-up resistors
	_SFR_IO8(port + 1) = (_SFR_IO8(port + 1) & ~(1<<pin)) | (io<<pin);		//set as input or output in DDR
	_SFR_IO8(port + 2) = (_SFR_IO8(port + 2) & ~(1<<pin)) | (arg<<pin);		//configure pull-up resistors for input or set initial value for output in PORT register
	
}

/*
Function:	gpioWrite
Arguments:	port		portX, X:A, B, C, D
			pin			0 - 7
			io			1, 0
*/
void gpioWrite(uint8_t port, uint8_t pin, uint8_t val){
	
	_SFR_IO8(port + 2) = (_SFR_IO8(port + 2) & ~(1<<pin)) | (val<<pin);		//write value to PORT register
	
}

/*
Function:	gpioRead
Arguments:	port		portX, X:A, B, C, D
			pin			0 - 7
Return:		1(HIGHI) / 0(LOW)
*/
uint8_t gpioRead(uint8_t port, uint8_t pin){
	
	return (_SFR_IO8(port) >> pin) & 0x01;					//read value from PIN register
	
}

/*
Function:	gpioToggle
Arguments:	port		portX, X:A, B, C, D
			pin			0 - 7
*/
void gpioToggle(uint8_t port, uint8_t pin){
	
	_SFR_IO8(port + 2) ^= 1<<pin;		//write value to PORT register
	
}

uint8_t gpioReadDebounce(uint8_t PB){
	static float PBtime[2] = {-1, -1};
	static uint8_t PBstate[2];
	uint8_t buff;
	
	if(PB)
		buff = gpioRead(BT2);
	else
		buff = gpioRead(BT1);
		
	if(!buff){
		
		PBtime[PB] = time();
		if(!PBstate[PB]){
			PBstate[PB] = 1;
			return 1;	
		}else{
			return 0;
		}
		
	}else{
		
		if(time() - PBtime[PB] > 0.05)
			PBstate[PB] = 0;
		return 0;
		
	}
	
}

/*
Function:	delay
Arguments:	time		delay in seconds
Remarks:	CKSEL3:0 should be set to 0001 for 1MHz Internal RC Oscillator
*/
void delay(float timeDelay){
	
	float timeBuff = time();						//initialize starting time
	while(time() - timeBuff < timeDelay);			//wait for specified time
	
}

/*
Function:	pwmInit
*/
void pwmInit(){ 
	
	
	gpioInit(0x10, 4, 1, 0);					//set PD4 as output
	gpioInit(0x10, 5, 1, 0);					//set PD5 as output
	
	
	TCNT1 = 0;
	OCR1A = 0;
	OCR1B = 0;
	TCCR1A = 0b10100011;						//fast PWM, non-inverting
	
	TCCR1B = 0b00001010;						//prescaler 8
	
		
}

/*
Function:	pwmSet
Arguments:	dutyCycle		0 - 100
*/
void pwmSet(uint8_t dutyCycle1, uint8_t dutyCycle2){
	
	
	OCR1A = (float)dutyCycle1 /100.0 * 1023.0 ;	//set output compare register value
	OCR1B = (float)dutyCycle2 /100.0 * 1023.0 ;	//set output compare register value
	
	
	

}
/*
Function:	twiInit
Remarks:	SCL freq	CLK / [16+2(TWBR)(4^prescaler in TWSR)] = 41.667kHz
			pull-up		internal resistors can be disabled and externally add 1.5kOhm or higher resistors
*/
void twiInit(){
	
	PORTC |= 0b00000011;	//enable pullup resistors
	TWBR = 1;				//set TWBR for SCL frequency = 41.667kHz
	TWSR = 0b00000000;		//prescaler = 1
	
}

/*
Function:	twiRead
Arguments:	add			slave address (without shifting 1 bit to the left)
			byte		number of bytes to read
			*data		pointer to data storage location, in uint8_t
Return:		1(error during TWI) or 0(no error)
*/
uint8_t twiRead(uint8_t add, uint8_t byte, uint8_t* data){
	uint8_t err = 0;
	
	TWCR = 0b11100100;					//send start
	while(!(TWCR&0b10000000));			//wait for start to send
	if((TWSR&0b11111000)!=Start)		//check status if start not sent
		err = 1;
	if(!err){
		TWDR = (add<<1)|0b00000001;		//set data
		TWCR = 0b11000100;				//send data
		while(!(TWCR&0b10000000));		//wait for data to send and receive ack
		if((TWSR&0b11111000)!=AddR)		//check if slave not respond
			err = 1;
	}
	if(!err){
		for(uint8_t i=byte; i>1; i--){
			TWCR = 0b11000100;					//continue to recieve data
			while(!(TWCR&0b10000000));			//wait for data done receiving
			if((TWSR&0b11111000)!=DatR){		//check if data corrupted
				err = 1;
				break;
			}
			*(uint8_t*)(data+byte-i) = TWDR;	//store data to pointer
		}
		if(!err){
			TWCR = 0b10000100;					//continue to recieve data with NACK
			while(!(TWCR&0b10000000));			//wait for data done receiving
			if((TWSR&0b11111000)!=DatRN)			//check if data corrupted
				err = 1;
			else
				*(uint8_t*)(data+byte-1) = TWDR;	//store data to pointer
		}
	}
	
	TWCR = 0b11010100;					//send stop
	return err;
}

/*
Function:	twiWrite
Arguments:	add			slave address (without shifting 1 bit to the left)
			byte		number of bytes to write
			*data		pointer to data to be sent, in uint8_t
Return:		1(error during TWI) or 0(no error)
*/
uint8_t twiWrite(uint8_t add, uint8_t byte, uint8_t* data){
	uint8_t err = 0;
	
	TWCR = 0b11100100;					//send start
	while(!(TWCR&0b10000000));			//wait for start to send
	if((TWSR&0b11111000)!=Start)		//check status if start not sent
		err = 1;
	if(!err){
		TWDR = (add<<1)&0b11111110;			//set address and write bit
		TWCR = 0b11000100;					//send data
		while(!(TWCR&0b10000000));			//wait for data to send and receive ack
		if((TWSR&0b11111000)!=AddW)			//check if add acked by slave
			err = 1;
	}
	if(!err){
		for(uint8_t i=byte; i>0; i--){
			TWDR = *(uint8_t*)(data+byte-i);	//set data
			TWCR = 0b11000100;					//send data
			while(!(TWCR&0b10000000));			//wait for data to send and receive ack
			if((TWSR&0b11111000)!=DatW){		//check if data acked by slave
				err = 1;
				break;
			}
		}
	}

	TWCR = 0b11010100;					//send stop
	return err;
}

/*
Function:	twiCombine
Arguments:	add			slave address (without shifting 1 bit to the left)
			dataW		byte to be written
			byteR		number of bytes to be read
			*dataR		pointer to data storage location, in uint8_t
Return:		1(error during TWI) or 0(no error)
*/
uint8_t twiCombine(uint8_t add, uint8_t dataW, uint8_t byteR, uint8_t* dataR){
	
	uint8_t err = 0;
	
	TWCR = 0b11100100;					//send start
	while(!(TWCR&0b10000000));			//wait for start to send
	if((TWSR&0b11111000)!=Start)		//check status if start not sent
		err = 1;
	if(!err){
		TWDR = (add<<1)&0b11111110;			//set address and write bit
		TWCR = 0b11000100;					//send data
		while(!(TWCR&0b10000000));			//wait for data to send and receive ack
		if((TWSR&0b11111000)!=AddW)			//check if add acked by slave
			err = 1;
	}
	if(!err){
		TWDR = dataW;						//set data
		TWCR = 0b11000100;					//send data
		while(!(TWCR&0b10000000));			//wait for data to send and receive ack
		if((TWSR&0b11111000)!=DatW)			//check if data acked by slave
			err = 1;
		
	}

	if(!err){
		TWCR = 0b11100100;					//send restart
		while(!(TWCR&0b10000000));			//wait for restart to send
		if((TWSR&0b11111000)!=RStart)		//check status if restart not sent
			err = 1;
	}
	if(!err){
		TWDR = (add<<1)|0b00000001;		//set data
		TWCR = 0b11000100;				//send data
		while(!(TWCR&0b10000000));		//wait for data to send and receive ack
		if((TWSR&0b11111000)!=AddR)		//check if slave not respond
		err = 1;
	}
	if(!err){
		for(uint8_t i=byteR; i>1; i--){
			TWCR = 0b11000100;					//continue to recieve data
			while(!(TWCR&0b10000000));			//wait for data done receiving
			if((TWSR&0b11111000)!=DatR){		//check if data corrupted
				err = 1;
				break;
			}
			*(uint8_t*)(dataR+byteR-i) = TWDR;	//store data to pointer
		}
		if(!err){
			TWCR = 0b10000100;					//continue to recieve data with NACK
			while(!(TWCR&0b10000000));			//wait for data done receiving
			if((TWSR&0b11111000)!=DatRN)			//check if data corrupted
				err = 1;
			else
				*(uint8_t*)(dataR+byteR-1) = TWDR;	//store data to pointer
		}
	}

	TWCR = 0b11010100;					//send stop
	return err;
	
}

/*
Function:	twiTest
Arguments:	min		smallest value of address to be tested
			max		largest value of address to be tested, max at 127
Return:		1(Slave is in address range) or 0(not in adress range)
*/
uint8_t twiTest(uint8_t min, uint8_t max){
	uint8_t val=0;
	
	while(min<=max){
		TWCR = 0b11100100;					//send start bit
		while(!(TWCR&0b10000000));			//wait for start to send
		if((TWSR&0b11111000)!=Start)		//check status if start not sent
			continue;
		TWDR = (min<<1)&0b11111110;			//set address and write bit
		TWCR = 0b11000100;					//send data
		while(!(TWCR&0b10000000));			//wait for data to send and receive ack
		if((TWSR&0b11111000)==AddW)			//check if add acked by slave
			val=1;
		TWCR = 0b11010100;					//send stop
		min++;
	}
	return val;
}

/*
Function:	uartInit
Remarks:	baudrate 9600
*/
void uartInit(){
	
	UCSRA = 0b00000010;			//double speed operation, less error
	UCSRC = 0b10000110;			//asynchronous, no parity, 1 stop bit,
	UBRRL = 0b00001100;			//baudrate = 9600
	
	UCSRB = 0b00011000;			//enable TX&RX, 8-bit frame
	
}

/*
Function:	uartTransmit
Arguments:	data		pointer to data to be transmitted
			byte		number of bytes to be transmitted
			wtime		maximum waiting time
Return:		1(Success) or 0(Error)
*/
uint8_t uartTransmit(uint8_t* data, uint8_t byte, float wtime){
	
	float timeBuff = time();
	for(uint8_t i=0; i<byte; i++){
		while(!(UCSRA & 0b00100000)){			//wait for UDR to be free
			if(time() - timeBuff > wtime)		//if max wait time exceeded
				return 1;
		}	
		UDR = *(data+i);						//send data
	}
	return 0;
	
}

/*
Function:	uartReceive
Arguments:	data		pointer to location where received data is stored
			byte		number of bytes to be received
			wtime		maximum waiting time
Return:		1(Success) or 0(Error)
*/
uint8_t uartReceive(uint8_t* data, uint8_t byte, float wtime){

	float timeBuff = time();
	for (uint8_t i=0; i<byte; i++){
		while(!(UCSRA & 0b10000000)){		//check RXC flag
			if(time() - timeBuff > wtime){	//if max wait time exceeded
				uint8_t noUse;
				while(UCSRA & 0b10000000)	//flush buffer
					noUse = UDR;
				return 1;
			}
		}
		if(UCSRA & 0b00011100){				//if error
			uint8_t noUse;
			while(UCSRA & 0b10000000)		//flush buffer
				noUse = UDR;
			return 1;
		}else{
			*data = UDR;					//set data received
		}
	}
	return 0;
}

/*
Function:	uartTransmitInt
Arguments:	val			around -60000 to 60000	
Return:		1(Success) or 0(Error)
*/
uint8_t uartTransmitInt(int32_t val){
	
	uint8_t neg = 0;
	int32_t buff;
	uint8_t msg[6] = {0, 0, 0, 0, 0, 0};
	uint8_t byte = 0;
	
	if(val<0){
		val = -val;
		neg = 1;
	}
		
	for(uint8_t i=0; i<5; i++){
		buff = val%10;
		msg[5-i] = buff+48;
		byte++;
		val -= buff;
		if(!val)
			break;
		val /= 10;	
	}
	
	if(neg)
		msg[5-byte] = 45;
	if(byte+neg != 6){
		for(uint8_t j = 0; j < byte+neg ; j++)
			msg[j] = msg[6-byte-neg+j];
	}
	
	return uartTransmit(msg, byte+neg, 0.15);
	
}


/*
Function:	EEWrite
Arguments:	add			base address in EEPROM
			num			number of bytes to be stored
			data		pointer to data to be stored
*/
void EEWrite(uint16_t add, uint16_t num, uint8_t* data){
	
	for(uint8_t i=0; i<num; i++){
		while(EECR & 0b00000010);
		EEAR = add + i;
		EEDR = *(data+i);
		cli();
		EECR = 0b00000100;
		EECR = 0b00000110;
		sei();
	}
	
}

/*
Function:	EERead
Arguments:	add			base address in EEPROM
			num			number of bytes to be read
			data		pointer to location where data to be read is stored
*/
void EERead(uint16_t add, uint16_t num, uint8_t* data){
	
	for(uint8_t i=0; i<num; i++){
		while(EECR & 0b00000010);
		EEAR = add + i;
		cli();
		EECR = 0b00000001;
		sei();
		*(data+i) = EEDR;
	}
	
}