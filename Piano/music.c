#include "music.h"
#include "input.h"
#include "LED.h"
#include "sound.h"
#include "ST7735.h"
struct Notes{						//class used to play notes
	int index;						//index of the note to play
	uint16_t* note;					//pointer to notes
}Note;

uint16_t music_Notes[]={C,D,E,F,A,B,C1};

const unsigned short Wave[64] = {			//wave array for DAC_OUT
	2048,2224,2399,2571,2737,2897,3048,3190,3321,3439,3545,3635,3711,3770,3813,3839,3848,3839,3813,3770,
	3711,3635,3545,3439,3321,3190,3048,2897,2737,2571,2399,2224,2048,1872,1697,1525,1359,1199,1048,906,775,
	657,551,461,385,326,283,257,248,257,283,326,385,461,551,657,775,906,1048,1199,1359,1525,1697,1872
};

void switch_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x10;        // 1) activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PE
  GPIO_PORTE_CR_R |= 0x1E;
  GPIO_PORTE_AMSEL_R &= 0;      // 4) disable analog function on PE1-PE4
	//GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFF0000F)+0x00000000;// configure PE1-4 as GPIO
	GPIO_PORTE_PCTL_R &= ~0x000FFFF0;
  GPIO_PORTE_PDR_R |= 0x1E;         // 5) pulldown for PE1-4
  GPIO_PORTE_DIR_R &= ~0x1E;         // 5) set direction to output
  GPIO_PORTE_AFSEL_R &= ~0x1E;      // 6) regular port function
	//GPIO_PORTF_PUR_R |= 0x0E;
  GPIO_PORTE_DEN_R |= 0x1E;         // 7) enable digital port
}

#define rest 10000000
uint16_t wave_index;
uint32_t load_Value=rest;

void notes_Init(void){	//initialize the class
	Note.index=0;
	Note.note=music_Notes;
	wave_index=0;
}
void screen_start(){		//initialize what to print to the LCD
	ST7735_SetCursor(0,0);
	ST7735_OutString("A    B    C    Flat");
	ST7735_SetCursor(0,2);
	ST7735_OutString("Notes being played");
}

void play_Note(uint8_t note){		//function to read what note to play and set timer0 to
	ST7735_SetCursor(0,3);	
	load_Value=0;
	uint32_t num=0;
	uint8_t flat=0;
	if((GPIO_PORTE_DATA_R&0x02)==0x02){		//reads what input is being pressed to see if notes are played or flats
		flat=1;
		ST7735_OutString("Flat ");
	}
	//if(note==0xFE){
	if((GPIO_PORTE_DATA_R&0x10)==0x10){			//checks to see if button for A is pressed
		if(flat==1){
			load_Value+=AF;
		}
		else{
			load_Value += A;
		}
			num+=1;
			ST7735_OutString("A     ");
	}
	if((GPIO_PORTE_DATA_R&0x08)==0x08){			//checks if button for B is pressed
		if(flat==1){
			load_Value+=BF;
		}
		else{
			load_Value += B;
		}
			num+=1;
			ST7735_OutString("B     ");
	}
	if((GPIO_PORTE_DATA_R&0x04)==0x04){			//checks if button for C is pressed
		if(flat==1){
			load_Value+=DF1;
		}
		else{
			load_Value += C;
		}
			num+=1;
			ST7735_OutString("C  ");
		}
	if(num==0){							//clears screen if nothing was pressed
		if(flat==0){
			load_Value=rest;
			ST7735_SetCursor(0,3);
			ST7735_OutString("                  ");
		}
	}
	if(num>0){						//set the timer to the proper note value
		TIMER0_TAILR_R=load_Value/num;
	}
	else{
		TIMER0_TAILR_R=load_Value;
	}
}

void Timer0A_Init(void){			//not 100hz, constantly changing
  volatile uint32_t delay;
  //DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****                        // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for interrupts
	//TIMER0_TAILR_R=A;
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****// Timer0A=priority 2
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R |= 1<<19;              // enable interrupt 19 in NVIC
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout	
	//play the instrument wave of the current instrument
	DAC_Out(Wave[wave_index]);
	wave_index+=1;										//increment wave index
	wave_index=wave_index%64;					//if wave index hits 64 then it will loop back to zero
}

void Timer2_Init(void){
	volatile uint32_t delay;
	SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer2A initialization ****                        // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 4999999;         // start value for interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****// Timer0A=priority 2
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x20000000; // top 3 bits
  NVIC_EN0_R |= 1<<23;              // enable interrupt 19 in NVIC
}

void Timer2A_Handler(void){
  TIMER2_ICR_R = 0x01;// acknowledge TIMER2A timeout
	//TIMER0_TAILR_R=A;
}
