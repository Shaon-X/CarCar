#include <avr/io.h>
#include <avr/interrupt.h>
#include "BasicFunction.c"

#define portA		0x19
#define portB		0x16
#define portC		0x13
#define portD		0x10

#define output		1
#define input		0

#define pullup		1

void systemTimeInit();
uint32_t tick();
float time();
void gpioInit(uint8_t port, uint8_t pin, uint8_t io, uint8_t arg);
void gpioWrite(uint8_t port, uint8_t pin, uint8_t val);
uint8_t gpioRead(uint8_t port, uint8_t pin);
void gpioToggle(uint8_t port, uint8_t pin);
void delay(float time);
void pwmInit(uint8_t dutyCycle);
void pwmSet(uint8_t dutyCycle);
void twiInit();
uint8_t twiRead(uint8_t add, uint8_t byte, uint8_t* ptr);
uint8_t twiWrite(uint8_t add, uint8_t byte, uint8_t* data);
uint8_t twiCombine(uint8_t add, uint8_t dataW, uint8_t byteR, uint8_t* dataR);
uint8_t twiTest(uint8_t min, uint8_t max);
void uartInit();
uint8_t uartTransmit(uint8_t* data, uint8_t byte, float wtime);
uint8_t uartReceive(uint8_t* data, uint8_t byte, float wtime);
uint8_t uartTransmitInt(int32_t val);
void EEWrite(uint16_t add, uint16_t num, uint8_t* data);
void EERead(uint16_t add, uint16_t num, uint8_t* data);