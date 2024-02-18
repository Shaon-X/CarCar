#include "Modules.c"






void ssegInit(uint8_t* port, uint8_t* pin, float inter);
void ssegEnable(uint8_t en);
void ssegSet(uint8_t digit, uint8_t val);
void ssegWrite(uint8_t digit, uint8_t val);
void ssegUpdate();
uint8_t TCSInit();
uint8_t TCSEnable();
uint8_t TCSDisable();
uint8_t TCSSleep();
uint8_t TCSRead(uint16_t* data);
uint8_t TCSRed(uint16_t *data);
uint8_t MPUInit();
void MPURead(float* data);
void motor(int8_t motorL, int8_t motorR);