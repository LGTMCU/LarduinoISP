// swd interface definition
#ifndef __LGT_SWDIF__
#define	__LGT_SWDIF__

#include <Arduino.h>

#ifdef __cplusplus
extern "C"{
#endif
	
#define LGTMCU_8F328D		0x1
#define LGTMCU_8F328P		0x2
#define LGTMCU_UNKNOWN		0x0

#define SWDIF_PIN		PINB
#define SWDIF_DIR		DDRB
#define SWDIF_PORT		PORTB
#define SWDIF_CLK		(1 << 5)	// PB5
#define SWDIF_DAT	  	(1 << 4)	// PB4
#define SWDIF_RSTN		(1 << 2)	// PB2

#define	SWC_CLR()	(SWDIF_PORT &= ~SWDIF_CLK)
#define	SWC_SET()	(SWDIF_PORT |= SWDIF_CLK)
#define	SWD_CLR()	(SWDIF_PORT &= ~SWDIF_DAT)
#define SWD_SET()	(SWDIF_PORT |= SWDIF_DAT)
#define SWD_IND()	(SWDIF_DIR &= ~SWDIF_DAT)
#define SWD_OUD()	(SWDIF_DIR |= SWDIF_DAT)

#define	RSTN_CLR()	(SWDIF_PORT &= ~SWDIF_RSTN)
#define RSTN_SET()	(SWDIF_PORT |= SWDIF_RSTN)
#define RSTN_IND()	(SWDIF_DIR &= ~SWDIF_RSTN)
#define RSTN_OUD()	(SWDIF_DIR |= SWDIF_RSTN)

// for 16MHz system clock
// delay used for swc generator
#ifndef NOP
#define NOP() asm volatile("nop")
#endif

#define SWD_Delay()	do {\
	NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); \
	NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); \
} while(0);

// reuse delay from Arduino
#ifndef delayus
#define delayus(n)	delayMicroseconds(n)
#endif
  
void SWD_init();
void SWD_exit();
void SWD_Idle(uint8_t);
void SWD_WriteByte(uint8_t, uint8_t, uint8_t);	
uint8_t SWD_ReadByte(uint8_t, uint8_t);

void SWD_SWDEN();
uint8_t SWD_UnLock();
void SWD_EEE_Write(uint16_t, uint16_t);
uint16_t SWD_EEE_Read(uint16_t);
	
void SWD_EEP_Write(uint32_t, uint16_t);
uint32_t SWD_EEP_Read(uint16_t);

#ifdef __cplusplus
}
#endif
	
#endif
