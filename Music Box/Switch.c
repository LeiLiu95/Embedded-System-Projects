#include <stdint.h>
#include <stdio.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Switch.h"
#include "music.h"

#define PE1   (*((volatile uint32_t *)0x4005C008))
#define PE2   (*((volatile uint32_t *)0x4005C010))
#define PE3   (*((volatile uint32_t *)0x4005C020))
#define PE4   (*((volatile uint32_t *)0x4005C040))
void EnableInterrupts();
void DisableInterrupts();
void DelayWait10ms(uint32_t n);

uint8_t playing=1;

void switch_Init(void){
	
	SYSCTL_RCGCGPIO_R |= 0x10;        // 1) activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTE_CR_R |= 0x0E;
  GPIO_PORTE_AMSEL_R &= 0;      // 4) disable analog function on PF2, PF4
	//GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFF0000F)+0x00000000;// configure PF2 as GPIO
	GPIO_PORTE_PCTL_R &= ~0x0000FFF0;
  GPIO_PORTE_PDR_R |= 0x0E;         // 5) pullup for PF4
  GPIO_PORTE_DIR_R &= ~0x0E;         // 5) set direction to output
  GPIO_PORTE_AFSEL_R &= ~0x0E;      // 6) regular port function
	//GPIO_PORTF_PUR_R |= 0x0E;
  GPIO_PORTE_DEN_R |= 0x0E;         // 7) enable digital port
}

void check_song(){
	if((GPIO_PORTE_DATA_R&0x02)==0x02){
		play_pause();
	}
	if((GPIO_PORTE_DATA_R&0x04)==0x04){
		rewind_button();
	}
	if(playing==1){
	}
	if((GPIO_PORTE_DATA_R&0x08)==0x08){
		instrument+=1;
		instrument=instrument%4;
	}
}

void play_pause(void){
	if(playing==0){
		playing=1;
		NVIC_ST_RELOAD_R=80000;
		EnableInterrupts();
	}
	else{
		playing=0;
		NVIC_ST_RELOAD_R=0;
		DisableInterrupts();
	}
}

void rewind_button(void){
	reset();
	playing=0;
}
	
void tempo_Change(void){
	
}
	
	