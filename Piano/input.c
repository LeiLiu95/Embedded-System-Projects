#include "input.h"
#include "LED.h"
#include "music.h"
#include "sound.h"



void static writedata(uint8_t c) {
  while((SSI2_SR_R&SSI_SR_TNF)==0){};   // wait until transmit FIFO not full
  int DC = 0;
  SSI2_DR_R = c;                        // data out
}

uint8_t sendAndRecieve(void){
	while((SSI2_SR_R&SSI_SR_TFE)==0){};
	SSI2_DR_R = 0x7F;
	while((SSI2_SR_R&SSI_SR_RNE)==0){};
	return SSI2_DR_R;
}

void input_Init(){
	SYSCTL_RCGCSSI_R |= 0x04;  // activate SSI2
  SYSCTL_RCGCGPIO_R |= 0x02; // activate port B
  while((SYSCTL_PRGPIO_R&0x02)==0){}; // allow time for clock to start

 /* // toggle RST low to reset; CS low so it'll listen to us
  // SSI0Fss is temporarily used as GPIO
  GPIO_PORTB_DIR_R |= 0x80;             // make PB7 out
  GPIO_PORTB_AFSEL_R &= ~0xF0;          // disable alt funct on PA3,6,7
  GPIO_PORTB_DEN_R |= 0xF0;             // enable digital I/O on PA3,6,7
                                        // configure PA3,6,7 as GPIO
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x00FF0FFF)+0x00000000;
  GPIO_PORTB_AMSEL_R &= ~0xF0;          // disable analog functionality on PB 4, 5, 6, 7*/


  // initialize SSI0
  GPIO_PORTB_AFSEL_R |= 0xF0;           // enable alt funct on PB 4,5,6, 7
				
 // GPIO_PORTB_DIR_R = 0x20;    
  GPIO_PORTB_DEN_R = 0xF0;             // enable digital I/O on PA2,3,5
                                        // configure PA2,3,5 as SSI
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x0000FFFF)+0x22220000;
  GPIO_PORTB_AMSEL_R &= ~0xF0;          // disable analog functionality on PA2,3,5
	
	//GPIO_PORTB_DATA_R |= 0x20;
  SSI2_CR1_R &= ~SSI_CR1_SSE;           // disable SSI
  SSI2_CR1_R &= ~SSI_CR1_MS;            // master mode
                                        // configure for system clock/PLL baud clock source
  SSI2_CC_R = (SSI2_CC_R&~SSI_CC_CS_M)+SSI_CC_CS_SYSPLL;
//                                        // clock divider for 3.125 MHz SSIClk (50 MHz PIOSC/16)
//  SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+16;
                                        // clock divider for 8 MHz SSIClk (80 MHz PLL/24)
                                        // SysClk/(CPSDVSR*(1+SCR))
                                        // 80/(10*(1+0)) = 8 MHz (slower than 4 MHz)
  SSI2_CPSR_R = (SSI2_CPSR_R&~SSI_CPSR_CPSDVSR_M)+10; // must be even number
  SSI2_CR0_R &= ~(SSI_CR0_SCR_M |       // SCR = 0 (8 Mbps data rate)
                  SSI_CR0_SPH |         // SPH = 0
                  SSI_CR0_SPO);         // SPO = 0
                                        // FRF = Freescale format
  SSI2_CR0_R = (SSI2_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
                                        // DSS = 8-bit data
  SSI2_CR0_R = (SSI2_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
  SSI2_CR1_R |= SSI_CR1_SSE;            // enable SSI
	
	writedata(0x7D);//set addr
	writedata(0x71);
	writedata(0x7E);//write
	writedata(0xFF);
	
	writedata(0x7D);//set addr
	writedata(0x72);
	writedata(0x7E);//write
	writedata(0xFF);
	
	writedata(0x7D);//set addr
	writedata(0x73);
	writedata(0x7E);//write
	writedata(0xFF);
	
	/*writedata(0x7F);//read
	writedata(0x7D);//set addr
	writedata(0x03);
	

	uint8_t input = sendAndRecieve();*/
	
	int delay = 0;
	
}



int current_Input(void){
	return 0;								//return what note should be played
}

