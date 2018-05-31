#include <stdint.h>
#include <stdio.h>
#include "music.h"
#include "dac.h"
#include "SysTick.h"

void EnableInterrupts();
void DisableInterrupts();



struct Song{
	int index;
	uint16_t* note;
	uint16_t* duration;
	int last;
}song;

uint16_t duration=0;
//									

//BF F F BF AF GF AF BF GF GF BF A G A rest rest
//BF F BF BF C d EF F F F GF AF BF BF AF GF AF GF F F
//EF F GF F EF DF EF F EF DF C D E G F1 F F F F F F F F F F
//BF F BF BF C D E F F F GF AF BF DF F EF B F GF BF A F F
//GF BF A F D EF GF F DF BF C D E G F1 F F F F F F F F F F

									//link to the past
uint16_t song1[]={BF, F, F, BF, AF, GF, AF, BF, GF, GF, BF, A, G, A, 0, 0, 
									BF, F, BF, BF, C, D1, EF1, F1, F1, F1, GF1, AF1, BF1, BF1, AF1, GF1, AF1, GF1, F1, F1,
									EF1, F1, GF1, F1, EF1, DF1, EF1, F1, EF1, DF1, C, D1, E1, G1, F1, F, F, F ,F ,F ,F ,F ,F ,F ,F, //60
									BF, F, BF, BF, C, D1, EF1, F1, F1, F1, GF1, AF1, BF1, DF2, C1, A1, F1, GF1, BF1, A1, F1, F1,
									GF1, BF1, A1, F1, D1, EF1, GF1, F1, DF1, BF, C, D1, E1, G1, F1, F, F, F, F, F, F, F, F, F, F,
	
									//Route 1  C#-DF F#-GF
									D1, E1, GF1, GF1, GF1, D1, E1, GF1, GF1, GF1,D1, E1, GF1, GF1, G1, DF1, B, GF1, E1, A, A, DF1, D1, E1, E1, E1, DF1, D1, E1, E1, E1, DF1, D1, //33
									E1, E1, GF1, E1, E1, GF1, D1, A, GF1, D1, E1, GF1, GF1, GF1, D1, E1, GF1, GF1,GF1, D1, E1, GF1, GF1, G1, DF1, B, GF1, E1, A, A, DF1, D1, E1, G1, GF1, E1, D1, DF1, DF1, DF1, //40
									B1, E1, GF1, GF1, G1, A1, A1, E1, D1, D2, DF2, B1, DF2, A1, GF1, D1, GF1, E1, A, C, GF1, G1, A1, A1, GF1, A1, D2, DF2, B1, DF1, G1,
									A1, D2, DF2, E2, D2, 0
}; 
uint16_t song1duration[]= {10, 2, 2, 2, 1, 1, 14, 10, 2, 2, 2, 1, 1, 14, 16, 16, 
														4, 6, 2, 1, 1, 1, 1, 8, 2, 2, 2, 2, 10, 2, 2, 2, 3, 1, 8, 4,
													3, 1, 8, 2, 2, 3, 1, 8, 2, 2, 3, 1, 8, 4, 2, 1, 1 , 2, 1, 1, 2, 1, 1 ,2 , 2,
													4, 6, 2, 1, 1, 1, 1, 8, 2, 2, 2, 2, 12, 4, 4, 8, 4, 12, 4, 4, 8, 4,
													12, 4, 4, 8, 4, 12, 4, 4, 8, 4, 3, 1, 8, 4, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 2,
	
	
	
													1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, //33
													2, 2, 1, 1, 1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 2, 1, 1, 2, 2,2,2, 2,2 ,2 ,2 , //40
													4, 4, 6, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, //31
													2, 2, 2, 2, 2, 16  //10
};
int last=217;

void init_Song(void){
	song.index=107;
	song.note=song1;
	song.duration=song1duration;
	song.last=last;
}

int wave_index=0;

const unsigned short Wave[64] = {
	2048,2224,2399,2571,2737,2897,3048,3190,3321,3439,3545,3635,3711,3770,3813,3839,3848,3839,3813,3770,
	3711,3635,3545,3439,3321,3190,3048,2897,2737,2571,2399,2224,2048,1872,1697,1525,1359,1199,1048,906,775,
	657,551,461,385,326,283,257,248,257,283,326,385,461,551,657,775,906,1048,1199,1359,1525,1697,1872
};

void reset(void){
	song.index=0;
}

void play_note(void){
}

void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  //DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****                        // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****// Timer0A=priority 2
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R |= 1<<19;              // enable interrupt 19 in NVIC
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
	//DAC_Out(song.note[song.index]);
	DAC_Out(Wave[wave_index]);
	wave_index+=1;
	wave_index=wave_index%64;
}

void Timer2_Init(void){
	volatile uint32_t delay;
 SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****                        // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 4999999;         // start value for 100 Hz interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****// Timer0A=priority 2
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x20000000; // top 3 bits
  NVIC_EN0_R |= 1<<23;              // enable interrupt 19 in NVIC
}

void Timer2A_Handler(void){
  TIMER2_ICR_R = 0x01;// acknowledge TIMER1A timeout
	
	if(duration==song.duration[song.index]){
		duration=0;
		song.index+=1;
		SysTick_Wait(100000);
		TIMER0_CTL_R |= TIMER_CTL_TAEN;
		if(song.index>song.last){
				song.index=0;
		}
		if(song.note[song.index] == 0){
			TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
		}
		if(song.note[song.index] == 0 && duration == (song.duration[song.index] - 1)){
			TIMER0_CTL_R |= TIMER_CTL_TAEN;
		}
		else{
			TIMER0_TAILR_R = song.note[song.index];
		}
		
	
	}
	duration+=1;
}

