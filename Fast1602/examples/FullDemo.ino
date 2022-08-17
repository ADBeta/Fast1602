#include <Fast1602.h>

#define lcdRS 8
#define lcdEN 9
//DataBus, D4 D5 D6 D7
const unsigned char lcdDB[4] = {10, 11, 12, 13};

Fast1602 LCD(lcdRS, lcdEN, lcdDB);

unsigned char customChar[8] = {0x00, 0x11, 0x00, 0x00, 0x11, 0x0E, 0x00};

void setup() {
	//Initialize the LCD
	LCD.init();
	
	//Print a string to the LCD
	LCD.printString("Hello World!!");
	
	//Create a custom char in RAM position 0
	LCD.createCustom(0, customChar);
	
	//Change position - this is due to the custom char messing with GCRAM and 
	//We need to tell it to go back to DRAM
	LCD.setPos(0, 1);
	
	//Then print our custom char
	LCD.printChar(0);
	
	//Shift cursor to the edge of the screen
	LCD.shiftCursor(14);
	
	//Print another custom char
	LCD.printChar(0);
	
	//Pause for dramatic effect...
	delay(2500);
	
	//Slowly move the screen all the way to the right
	for(uint8_t x = 0; x < 16; x++){
		LCD.shiftScreen(1);
		delay(250);
	}
	
	//Now that the data is off the edge of the screen, clear the RAM
	LCD.clear();
}

void loop() {
	//Start printing all the chars to the screen
	
	//Current char to be printed to screen. Starts at '!'
	static uint8_t charIndex = 0x21;
	//Keep track of where we are on the LCD
	static uint8_t currentX = 0;
	static uint8_t currentY = 0;
	
	//Print the current char in index to the screen.
	LCD.printChar(charIndex);
	//Incriment the index position for a new char and the tracked position of X
	++charIndex;
	++currentX;
	
	if(currentX == 16) {
		//Reset X
		currentX = 0;
		
		//Move to the correct line aka Y position
		if(currentY == 0) {
			currentY = 1;
		} else {
			currentY = 0;
		}
		
		//Set the position to the values we just selected
		LCD.setPos(currentX, currentY);		
	}
	
	//Delay to make it look nice
	delay(100);
	
}
