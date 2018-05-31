#include <stdint.h>
#include <stdio.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "sound.h"
#include "music.h"
#include "input.h"

void DisableInterrupts(void);
void EnableInterrupts(void); 
void DelayWait10ms(uint32_t n);

int main(void){
  PLL_Init(Bus50MHz);                   // 80 MHz
	input_Init();		
	ST7735_InitR(INITR_REDTAB);				//initialize the st7735 screen
  ST7735_FillScreen(ST7735_BLACK);  // set screen to black
	ST7735_SetTextColor(ST7735_WHITE);	//set text color to whie
	dac_init(0);						//initialize DAC
	notes_Init();						//initialize the notes
	switch_Init();					//initialize buttons
	Timer2_Init();					//initialize timer interrupts
	Timer0A_Init();
	screen_start();					//print info to LCD
	
	while(1){
		play_Note(0);
	}
}
