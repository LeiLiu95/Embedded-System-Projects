#include <stdint.h>
#include <stdio.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Switch.h"
#include "music.h"
#include "dac.h"
#include "SysTick.h"

void DisableInterrupts(void);
void EnableInterrupts(void); 
void DelayWait10ms(uint32_t n);

int main(void){
  PLL_Init(Bus50MHz);                   // 50 MHz
	//ST7735_InitR(INITR_REDTAB);
	switch_Init();
	dac_init(0);
	SysTick_Init();
	init_Song();
	DisableInterrupts();
	Timer0A_Init100HzInt();
	Timer2_Init();
	EnableInterrupts();
	while(1){
		check_song();
	}
}