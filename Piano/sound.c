#include "sound.h"
#include "LED.h"
#include "music.h"
#include "input.h"

void dac_init(uint16_t initialData) {		//DAC initialization
	SYSCTL_RCGCSSI_R |= 0x02;
	SYSCTL_RCGCGPIO_R |= 0x08;
	while((SYSCTL_PRGPIO_R & 0x08) == 0){};
	GPIO_PORTD_AFSEL_R |= 0x0B;
	GPIO_PORTD_DEN_R |= 0x0B;
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R & 0xFFFF0F00) + 0x00002022;
	GPIO_PORTD_AMSEL_R = 0;
	SSI1_CR1_R = 0x0;
	SSI1_CPSR_R = 0x02;
	SSI1_CR0_R &= ~(0x0000FFF0);
	SSI1_CR0_R |= 0x0F;
	SSI1_DR_R = initialData;
	SSI1_CR1_R |= 0x02;
}

void DAC_Out(uint16_t code) {
	while((SSI1_SR_R & 0x02) == 0){};
	SSI1_DR_R = code;
}
