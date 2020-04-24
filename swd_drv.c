// implement of swd interface

#include "swd_drv.h"

void SWD_init()
{
	// set to output
	SWDIF_DIR |= (SWDIF_CLK | SWDIF_DAT);
	// clear output
	SWD_CLR();
	SWC_CLR();
}

void SWD_exit()
{
	SWDIF_DIR &= ~(SWDIF_CLK | SWDIF_DAT);
}

void SWD_WriteByte(uint8_t start, uint8_t data, uint8_t stop)
{
	volatile uint8_t cnt;
	
	if(start) {
		SWC_CLR();
		SWD_Delay();
		SWD_CLR();
		SWD_Delay();
		SWC_SET();
		SWD_Delay();
	}
	
	// send data
	for(cnt = 0; cnt < 8; cnt++)
	{
		SWC_CLR();
		if(data & 0x1) SWD_SET();
		else SWD_CLR();
		SWD_Delay();
		data >>= 1;
		SWC_SET();
		SWD_Delay();
	}
	
	SWC_CLR();
	if(stop) SWD_SET();
	else SWD_CLR();

	SWD_Delay();
	SWC_SET();
	SWD_Delay();
}

uint8_t SWD_ReadByte(uint8_t start, uint8_t stop)
{
	volatile uint8_t cnt;
	volatile uint8_t bRes = 0;
	
	if(start)
	{
		SWC_CLR();
		SWD_CLR();
		SWD_Delay();
		SWC_SET();
		SWD_Delay();		
	}
	
	SWD_IND();
	//SWD_Delay();
	for(cnt = 0; cnt < 8; cnt++)
	{
		bRes >>= 1;
		SWC_CLR();
		SWD_Delay();
		if(SWDIF_PIN & SWDIF_DAT)
			bRes |= 0x80;

		SWC_SET();
		SWD_Delay();
	}
	
	SWD_OUD();
	
	SWC_CLR();
	if(stop) SWD_SET();
	else SWD_CLR();

	SWD_Delay();
	SWC_SET();
	SWD_Delay();
	
	return bRes;
}

void SWD_Idle(uint8_t cnt)
{
	volatile uint8_t i;

	SWD_SET();
	
	for(i = 0; i < cnt; i++)
	{
		SWC_CLR();
		SWD_Delay();
		SWC_SET();
		SWD_Delay();
	}
}

void SWD_ReadSWDID(char *pdata)
{	
	SWD_WriteByte(1, 0xae, 1);
	SWD_Idle(8);
	pdata[0] = SWD_ReadByte(1, 0);
	pdata[1] = SWD_ReadByte(0, 0);
	pdata[2] = SWD_ReadByte(0, 0);
	pdata[3] = SWD_ReadByte(0, 1);
	SWD_Idle(8);
}

void SWD_SWDEN()
{
	SWD_WriteByte(1, 0xd0, 0);
	SWD_WriteByte(0, 0xaa, 0);
	SWD_WriteByte(0, 0x55, 0);
	SWD_WriteByte(0, 0xaa, 0);
	SWD_WriteByte(0, 0x55, 1);
	SWD_Idle(8);
}

void SWD_UnLock0()
{
	SWD_WriteByte(1, 0xf0, 0);
	SWD_WriteByte(0, 0x54, 0);
	SWD_WriteByte(0, 0x51, 0);
	SWD_WriteByte(0, 0x4a, 0);
	SWD_WriteByte(0, 0x4c, 1);
	SWD_Idle(8);
}

void SWD_UnLock1()
{
	SWD_WriteByte(1, 0xf0, 0);
	SWD_WriteByte(0, 0x23, 0);
	SWD_WriteByte(0, 0x50, 0);
	SWD_WriteByte(0, 0x49, 0);
	SWD_WriteByte(0, 0x2d, 1);
	SWD_Idle(8);
}

void SWD_EEE_CSEQ(uint8_t ctrl, uint16_t addr)
{
	SWD_WriteByte(1, 0xb2, 0);
	SWD_WriteByte(0, (addr & 0xff), 0);
	SWD_WriteByte(0, ((ctrl & 0x3) << 6) | ((addr >> 8) & 0x3f), 0);
	SWD_WriteByte(0, (0xC0 | (ctrl >> 2)), 1);
	SWD_Idle(8);
}

void SWD_EEE_DSEQ(uint16_t data)
{
	SWD_WriteByte(1, 0xb2, 0);
	SWD_WriteByte(0, (data & 0xff), 0);
	SWD_WriteByte(0, ((data >> 8) & 0xff), 1);
	SWD_Idle(8);
}

void SWD_EEP_DSEQ(uint32_t data)
{
	SWD_WriteByte(1, 0xb2, 0);
	SWD_WriteByte(0, (data & 0xff), 0);
	SWD_WriteByte(0, ((data >> 8) & 0xff), 0);
	SWD_WriteByte(0, ((data >> 16) & 0xff), 0);
	SWD_WriteByte(0, ((data >> 24) & 0xff), 1);
	SWD_Idle(8);
}

uint8_t SWD_EEE_GetBusy()
{
	uint8_t res = 0;
	
	SWD_WriteByte(1, 0xaa, 1);
	SWD_Idle(8);
	SWD_ReadByte(1, 0);
	SWD_ReadByte(0, 0);
	res = SWD_ReadByte(0, 1);
	SWD_Idle(8);
	
	return res & 0x1;
}

void SWD_EEE_UnlockTiming()
{
	volatile uint8_t ib;
	volatile uint8_t timout = 0x1f;
	
	SWD_EEE_CSEQ(0x00, 0x00);
	delay(5);
	SWD_EEE_CSEQ(0x08, 0x00);
	SWD_EEE_CSEQ(0x88, 0x00);
	SWD_EEE_CSEQ(0x02, 0x00);
	
	do {
		delayus(50);
		ib = SWD_EEE_GetBusy();
		--timout;
	} while(ib == 1 && timout > 0);
}

void SWD_EEP_UnlockTiming()
{
	SWD_EEE_CSEQ(0x00, 0x00);
	delayus(100);
	SWD_EEE_CSEQ(0x98, 0x00);
  delayus(50);
	SWD_EEE_CSEQ(0x9a, 0x00);
	delay(30);
	SWD_EEE_CSEQ(0x8a, 0x00);
	delayus(300);
	SWD_EEE_CSEQ(0x88, 0x00);
  delayus(100);
	SWD_EEE_CSEQ(0x00, 0x00);
  delayus(100);

}

uint16_t SWD_EEE_Read(uint16_t addr)
{
	uint8_t hbyte, lbyte;
	
	SWD_EEE_CSEQ(0x00, addr);
	SWD_EEE_CSEQ(0xa0, addr);
	
	SWD_WriteByte(1, 0xaa, 1);
	lbyte = SWD_ReadByte(1, 0);
	hbyte = SWD_ReadByte(0, 1);
	SWD_Idle(10);
	SWD_EEE_CSEQ(0x00, addr);

	return ((uint16_t)hbyte << 8) | lbyte;
}

uint32_t SWD_EEP_Read(uint16_t addr)
{
	uint32_t dw; 

	SWD_EEE_CSEQ(0xc0, addr);
	SWD_EEE_CSEQ(0xe0, addr);

	SWD_WriteByte(1, 0xaa, 1);
	dw = SWD_ReadByte(1, 0);
	dw |= (uint32_t)SWD_ReadByte(0, 0) << 8;
	dw |= (uint32_t)SWD_ReadByte(0, 0) << 16;
	dw |= (uint32_t)SWD_ReadByte(0, 1) << 24;
	SWD_Idle(10);
	SWD_EEE_CSEQ(0x00, addr);
	SWD_Idle(10);

	return dw;
}

void SWD_EEE_Write(uint16_t data, uint16_t addr)
{
	volatile uint8_t ib;
	volatile uint8_t timout = 0x1f;
	
	SWD_EEE_CSEQ(0x00, addr);
	SWD_EEE_DSEQ(data);
	SWD_EEE_CSEQ(0x04, addr);
	SWD_EEE_CSEQ(0x84, addr);
	SWD_EEE_CSEQ(0x02, addr);
	
	do {
		delayus(50);
		ib = SWD_EEE_GetBusy();
		--timout;
	} while(ib == 1 && timout > 0);
	
	SWD_EEE_CSEQ(0x00, addr);
}

void SWD_EEP_Write(uint32_t data, uint16_t addr)
{
	SWD_EEE_CSEQ(0x84, addr);
	SWD_EEP_DSEQ(data);
	SWD_EEE_CSEQ(0x86, addr);
	delayus(2);
	SWD_EEE_CSEQ(0xc6, addr);
	delayus(12);
	SWD_EEE_CSEQ(0x86, addr);
	SWD_EEE_CSEQ(0x82, addr);
	SWD_EEE_CSEQ(0x80, addr);
	SWD_EEE_CSEQ(0x00, addr);
	delayus(1);
}

uint8_t SWD_UnLock()
{
	char swdid[4];

	uint8_t mtype = LGTMCU_UNKNOWN;
	
	SWD_Idle(80);
	SWD_SWDEN();	 
	SWD_ReadSWDID(swdid);

	if((uint8_t)swdid[1] == 0xA2) { 
		mtype = LGTMCU_8F328P;	// 328P
	} else if((uint8_t)swdid[1] == 0xA1) {
		mtype = LGTMCU_8F328D;	// 328D
	} else {
		return LGTMCU_UNKNOWN;	// unknown
	}

	if(((uint8_t)swdid[0] & 0x3f) == 0x3f)
		return (mtype | 0x80);
  
	SWD_UnLock0();
	
	if(mtype == LGTMCU_8F328P)
	{
		SWD_EEP_UnlockTiming();
	}
	else
	{
		SWD_EEE_UnlockTiming();
	}
  
	SWD_UnLock1();
	delayus(100);
	SWD_UnLock0();
	delayus(100);
	SWD_UnLock1();	
  
 	SWD_ReadSWDID(swdid);
  
 	if(((uint8_t)swdid[0] & 0x3f) == 0x3f)
    		return (mtype | 0x80);

 	return 0; 
}

