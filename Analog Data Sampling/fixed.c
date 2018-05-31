// filename ******** fixed.h ************** 
// possible header file for Lab 1 
// feel free to change the specific syntax of your system
//All credit to Valvano
//Jacob Perlmutter and Lei Liu
//August 31, 2016

#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"
#include "PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include "fixed.h"

/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    " 2.345"  
 -8100    "-8.100"
  -102    "-0.102" 
    31    " 0.031" 
-12345    " *.***"
 */ 


	void ST7735_sDecOut3(int32_t n){
	int32_t output = n;
	uint16_t magnitudeCTR = 10000;	
	//Overflow, Negatives, and Spacing Corrections
	if(n > 99999 || n < -99999){ST7735_OutString(" *.****");/**/return;}
	//if(n < 0){ST7735_OutChar('-');/**/output*= -1;}
	else{ST7735_OutChar(' ');}
	
	while(magnitudeCTR>=1){
		if(magnitudeCTR==1000){ST7735_OutChar('.');}
		ST7735_OutUDec(output/magnitudeCTR);
		//Update
		output%=magnitudeCTR;
		magnitudeCTR/=10;
	}
}
	



/**************ST7735_uBinOut8***************
 unsigned 32-bit binary fixed-point with a resolution of 1/256. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 256000, it signifies an error. 
 The ST7735_uBinOut8 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
     0	  "  0.00"
     2	  "  0.01"
    64	  "  0.25"
   100	  "  0.39"
   500	  "  1.95"
   512	  "  2.00"
  5000	  " 19.53"
 30000	  "117.19"
255997	  "999.99"
256000	  "***.**"
*/
void ST7735_uBinOut8(uint32_t n)
{
	//Overflow Correction
	if(n > 255999) {ST7735_OutString("***.**");/**/return;}
	
	uint32_t magnitudeCTR = 100000;
	uint32_t output = (n*1000)/256; //Normalized to Decimal
	
	while(magnitudeCTR>1){
	if((output/100000 == 0 && magnitudeCTR == 100000) || (output/10000 == 0 && magnitudeCTR == 10000)){ST7735_OutChar(' ');} //Number too low
	else{
		if(magnitudeCTR == 100){ST7735_OutChar('.');} 
		if(magnitudeCTR == 100000 || magnitudeCTR == 10000){//Prevents 101.56 from becoming 1 1.56
			ST7735_OutUDec(output/magnitudeCTR);
			output%= magnitudeCTR; //Update
			magnitudeCTR/=10;
		}
		ST7735_OutUDec(output/magnitudeCTR);
	}//Update
	output%= magnitudeCTR;
	magnitudeCTR/=10;
	}
} 

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
#define TITLEHEIGHT 26
#define SCREENHEIGHT 160
#define GRAPHICWIDTH 128
#define GRAPHICHEIGHT 128

int32_t static dimensionalData[4]; // Used to create a surface which will be mapped to the ST7735
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
	ST7735_FillScreen(ST7735_BLACK); //BLACK 
	ST7735_FillRect(0, TITLEHEIGHT, GRAPHICWIDTH, SCREENHEIGHT-TITLEHEIGHT, ST7735_WHITE);
	ST7735_SetCursor(0, 1);
	ST7735_OutString(title); 
	dimensionalData[0] = minX;
	dimensionalData[1] = maxX - minX; // width
	dimensionalData[2] = maxY;
	dimensionalData[3] = minY - maxY; // height
}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){
	uint32_t counter = 0;
	while(counter < num){
		// Normalized Coordinate between 0 and 1 and multiplied by pixel width/height
		uint8_t mappedX = GRAPHICWIDTH*(bufX[counter]-dimensionalData[0])/(dimensionalData[1]);
		uint8_t mappedY = (GRAPHICHEIGHT*(bufY[counter]-dimensionalData[2])/(dimensionalData[3]))+TITLEHEIGHT;
		// 2 x 2 Pixel Thickness Plotter
		ST7735_DrawPixel(mappedX-1, mappedY, ST7735_BLUE);		/**/	ST7735_DrawPixel(mappedX, mappedY, ST7735_BLUE);
		ST7735_DrawPixel(mappedX-1, mappedY-1, ST7735_BLUE);	/**/	ST7735_DrawPixel(mappedX, mappedY-1, ST7735_BLUE);
		counter ++;
	}
	
}
