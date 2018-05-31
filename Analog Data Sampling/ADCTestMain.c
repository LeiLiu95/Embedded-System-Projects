// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdint.h>
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"
#include "fixed.h"
#include "PLL.h"
#include "ST7735.h"

#define PF4  					  (*((volatile uint32_t *)0x40025040))
#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
#define DUMPLENGTH	100
#define MAXHISTORY  10
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void DelayWait10ms(uint32_t n);
void pmf(void);
void timing(void);

volatile uint32_t ADCvalue;
uint8_t screen = 0;
uint8_t trigger = 0;
uint16_t globalDumpIndex=0;
uint16_t history = 0;
uint32_t time_Dump[DUMPLENGTH];
uint32_t dataValue_Dump[MAXHISTORY*DUMPLENGTH];
uint32_t pmffreq[4096];

// ***************** TIMER0_Init ****************
// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.
void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
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

void ST7735_drawline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){  //function that is used to draw a line with a slope on the LCD
	float slope = ((y1 - y2)/(x2 - x1));
	for(int x = x1; x < x2; x++){
		ST7735_DrawPixel(x, (int)(slope*(x1 - x)+ y1), ST7735_WHITE);
	}
}

// ***************** TIMER1_Init ****************
//TIMER1 interrupts to run user task periodically
void Timer1_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = 0xFFFFFFFE;  // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
  PF2^=0x04;
	PF2^=0x04;
	ADCvalue = ADC0_InSeq3();
	PF2^=0x04;
	if(globalDumpIndex == 0) {globalDumpIndex = DUMPLENGTH*(history%MAXHISTORY);}	//Begins dumping data into the dump
	if(!trigger){
		if(globalDumpIndex < DUMPLENGTH + DUMPLENGTH*(history%MAXHISTORY)){//When the array is not full yet it will record the time and the 
			time_Dump[globalDumpIndex%DUMPLENGTH]=TIMER1_TAR_R; //adcvalue for each interrupt that occurs in Timer1
			dataValue_Dump[globalDumpIndex]=ADCvalue;
			globalDumpIndex++;																//increases counter for the dump
		}
		else{
			trigger = 1;
		}
	}
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = 1;// acknowledge TIMER1A timeout
}

// ***************** TIMER2_Init ****************
//TIMER2 interrupts to run user task periodically
//to introduce jitter
void Timer2_Init(void){
	volatile uint32_t delay;
 SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****                        // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 7998;         // start value for 100 Hz interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****// Timer0A=priority 2
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x20000000; // top 3 bits
  NVIC_EN0_R |= 1<<23;              // enable interrupt 19 in NVIC
}
void Timer2A_Handler(void){
  TIMER2_ICR_R = 0x01;// acknowledge TIMER1A timeout
}
//  *************PMF**************
//PMF draws a pmf on the ST7735
void pmf(void)
{
	uint16_t min = 4095; 
	uint16_t max = 0;
	uint16_t dump_Index = 0;
	//over all data: min, max, and construct pmffreq
	if(history < MAXHISTORY){									//if there is more data than available space, then the max and min variance will be found
		while(dump_Index<DUMPLENGTH +history*DUMPLENGTH){
			uint16_t counter = dataValue_Dump[dump_Index];
			if(min > dataValue_Dump[dump_Index] && dataValue_Dump[dump_Index] < 4096) {min = dataValue_Dump[dump_Index];}
			if(max < dataValue_Dump[dump_Index] && dataValue_Dump[dump_Index] < 4096) {max = dataValue_Dump[dump_Index];}
			pmffreq[counter]++;
			dump_Index++;
		}	
	}
	else{																			//stores the data for each value and tracks it to be printed onto the LCD
		while(dump_Index<MAXHISTORY*DUMPLENGTH){
			uint16_t counter = dataValue_Dump[dump_Index];
			if(min > dataValue_Dump[dump_Index] && dataValue_Dump[dump_Index] < 4096) {min = dataValue_Dump[dump_Index];}
			if(max < dataValue_Dump[dump_Index] && dataValue_Dump[dump_Index] < 4096) {max = dataValue_Dump[dump_Index];}
			pmffreq[counter]++;
			dump_Index++;
		}
	}
	if(min > 1){min-=2;}
	if(max < 4092){max+=4;}
	uint16_t width = max - min;
	//data processing 
	uint16_t maxtemp = 0;			//processes the data to scale properly so graphic can be printed in terms of scaling
	uint32_t weightedAVG = 0;
	uint16_t sum = 0;
	for(int x = min; x< max; x++){
		if(pmffreq[maxtemp] < pmffreq[x]) {maxtemp = x;} //ADC input with most occurances. Used for scaling y axis.
		weightedAVG += x*pmffreq[x]; //Calculating simple weighted average 
		sum += pmffreq[x];
	}	
	weightedAVG=weightedAVG*10/sum;
	uint16_t maxtempval = pmffreq[maxtemp];
	ST7735_FillRect(0, 30, 128, 130, ST7735_BLACK);
	//Display Voltage
	ST7735_SetCursor(4,2);
	ST7735_sDecOut3(weightedAVG*100000/124121);
	ST7735_OutString(" Volts");
	//draw the grid
	ST7735_SetCursor(0,9);
	ST7735_OutUDec(maxtempval/2);
	ST7735_SetCursor(0,3);
	ST7735_OutUDec(maxtempval);
	ST7735_SetCursor(1,15);
	ST7735_OutUDec(min);
	ST7735_SetCursor(16,15);
	ST7735_OutUDec(max);
	ST7735_DrawFastVLine(20-50/width, 40,100, ST7735_WHITE);
	ST7735_DrawPixel(20-50/width , 91, ST7735_WHITE);
	//Draw the data
	for(int x = min; x < max; x++){		//function that draws the graph for pmf data
		ST7735_DrawFastVLine(((x-min)*100/width) +21, 141,2, ST7735_WHITE);
		for(int thickX = ((x-min)*100/width) - 50/width -1; thickX <((x-min)*100/width)+1 + 50/width; thickX++){
			ST7735_DrawFastVLine(thickX+21, 140-(100*pmffreq[x]/maxtempval), (100*pmffreq[x]/maxtempval)+1, ST7735_WHITE);
		}
		pmffreq[x]= 0;
	}
	history=(history + 1);
	globalDumpIndex = 0;	
}
//***************Timing******************
//timing calculates and displays the maximum
//jitter in units of 12.5 nano seconds
void timing(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_SetCursor(0,0);
	ST7735_OutString("Sampling Timer0"); 
	ST7735_SetCursor(0,1);
	if(screen == 0){ST7735_OutString("One smplng intrrupt");}
	if(screen == 1){ST7735_OutString("Two smplng intrrupt");}
	 

	int x = 1; int y = 3;
	int dump_Index = 0;
	
	
	while(dump_Index<DUMPLENGTH-1){						//function that keeps track of dump index
		time_Dump[dump_Index]-=time_Dump[dump_Index+1];
		dump_Index++;
	}
	uint32_t max = 0; uint32_t min = 0xFFFFFFFF;
	for(int x = 0; x < DUMPLENGTH - 2; x++)	{ //method that calculates the jitter to be displayed onto the LCD
		int jitter = time_Dump[x] -= time_Dump[x + 1];
		if(jitter < 0) {jitter*=-1;}
		if(max < jitter){ max = jitter;}
		if(min > jitter){ min = jitter;}
	}
	
	globalDumpIndex = 0;
	ST7735_SetCursor(0,3);
	ST7735_OutString("Units of 12.5ns");
	ST7735_SetCursor(0,4);
	ST7735_OutString("Max Jitter: ");
	ST7735_OutUDec(max); 	
}




int main(void){
  PLL_Init(Bus80MHz);                   // 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
  
	Timer1_Init();												// Initialize Timer1A without interrupts
//	Timer2_Init();
	Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
	
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x16;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x16;             // enable digital I/O on PF2, PF1                   
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF0F00F)+0x00000000;// configure PF2 as GPIO
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
	GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
	
  PF2 = 0;                      // turn off LED
	ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(ST7735_BLACK);  // set screen to black
	ST7735_SetTextColor(ST7735_YELLOW);
  EnableInterrupts();
  while(1){
		PF1 ^= 0x02;  // toggles when running in main
		if(PF4==0x00){
			while(PF4==0x00){}
			screen = (screen+1)%7;
			if(screen == 0){
				ST7735_FillScreen(ST7735_BLACK); 
				history = 0;
			}
			if(screen == 1){Timer2_Init();}
			if(screen > 1 && screen < 6){
				ST7735_FillScreen(ST7735_BLACK);
				ST7735_SetCursor(1,0); ST7735_OutString("ADC Input Histogram");
				ST7735_SetCursor(1,1);
				if(screen == 2){ADC0_SAC_R = 0x00; TIMER2_CTL_R &= ~TIMER_CTL_TAEN; ST7735_OutString("Hardware AVG: none");}	//Sets avging to none and displays data onto the screen
				if(screen == 3){ADC0_SAC_R = 0x02; ST7735_OutString("Hardware AVG: 4x");}	//4x avging that will display data onto the screen
				if(screen == 4){ADC0_SAC_R = 0x04; ST7735_OutString("Hardware AVG: 16x");}	//16x avging
				if(screen == 5){ADC0_SAC_R = 0x06; ST7735_OutString("Hardware AVG: 64x");}	//64x avging
			}
			if(screen == 6){	ST7735_FillScreen(ST7735_BLACK);  ST7735_drawline(20, 20, 60, 80);}	//method to draw a line onto the screen
		}
		
		if(screen < 2 && trigger == 1){	//keep track of trigger and screen values to know which screen to print
			timing();
			trigger = 0;
		}
		if(screen > 1 && trigger == 1 && screen < 6){ //display pmf if displaying any of the pmf screens
			pmf(); 
			trigger = 0;
		}
	}
}