/* Faster Library for the 1602 LCD using assembly. please refer to 
 * Fast1602LCD.h for more documentation
 *
 * ADBeta
*/

#include "Arduino.h"
#include "Fast1602.h"

/** Fast1602 Constructors *****************************************************/
//TODO make this less repeated and more standalone
//without read support
Fast1602::Fast1602(const uint8_t RS, const uint8_t EN, const uint8_t *DB) {
	//Set read flag
	readEn = 0;

	/* Convert the arduino pins to asm pins */
	hw_RSasm = digitalPinToBitMask(RS);
	hw_ENasm = digitalPinToBitMask(EN);
	
	/* Convert the arduino pin to port */
	hw_RSport = portOutputRegister(digitalPinToPort(RS));
	hw_ENport = portOutputRegister(digitalPinToPort(EN));
	
	/* Do the pin and port conversion on the DB pins in one loop. */
	for(uint8_t i = 0; i < 4; i++) {
		uint8_t cPin = DB[i];
		//ASM pin
		hw_DBasm[i] = digitalPinToBitMask(cPin);
		//Write port
		hw_DBport[i] = portOutputRegister(digitalPinToPort(cPin));
		//DDR register
		hw_DBdir[i] = portModeRegister(digitalPinToPort(cPin));
		//Read port	
		hw_DBread[i] = portInputRegister(digitalPinToPort(cPin));
	}
	
	//Set the Data Bus to output
	setOutput();
	
	//Write EN High before setting OUTPUT to prevent locking up on a reset
	*hw_ENport |= hw_ENasm;
	
	//Use the built in pinMode functions to save some code and variables.
	pinMode(RS, OUTPUT);
	pinMode(EN, OUTPUT);
}

//with read support
Fast1602::Fast1602(const uint8_t RS, const uint8_t RW, const uint8_t EN, 
	const uint8_t *DB) 
{ 
	//Set read flag
	readEn = 1;
	
	/* Convert the arduino pins to asm pins */
	hw_RSasm = digitalPinToBitMask(RS);
	hw_RWasm = digitalPinToBitMask(RW);
	hw_ENasm = digitalPinToBitMask(EN);
	
	/* Convert the arduino pin to port */
	hw_RSport = portOutputRegister(digitalPinToPort(RS));
	hw_RWport = portOutputRegister(digitalPinToPort(RW));
	hw_ENport = portOutputRegister(digitalPinToPort(EN));
	
	/* Do the pin and port conversion on the DB pins in one loop. */
	for(uint8_t i = 0; i < 4; i++) {
		uint8_t cPin = DB[i];
		//ASM pin
		hw_DBasm[i] = digitalPinToBitMask(cPin);
		//Write port
		hw_DBport[i] = portOutputRegister(digitalPinToPort(cPin));
		//DDR register
		hw_DBdir[i] = portModeRegister(digitalPinToPort(cPin));
		//Read port	
		hw_DBread[i] = portInputRegister(digitalPinToPort(cPin));
	}
	
	//Set the Data Bus to output
	setOutput();
	
	//Write EN High before setting OUTPUT to prevent locking up on a reset
	*hw_ENport |= hw_ENasm;
	
	//Use the built in pinMode functions to save some code and variables.
	pinMode(RS, OUTPUT);
	pinMode(RW, OUTPUT);
	pinMode(EN, OUTPUT);
}

void Fast1602::init() {
	//LCD Init delay 
	delay(15);
	
	txCmd(0x02); //Initializes LCD in 4 bit mode
	
	//TODO Make defaults and allow config before init()
	confFunction(0, 1, 0); //4Bit, 2 line, 5x8
	confEntryMode(1, 0); //Cursor Shift right, not screen
	confDispMode(1, 0, 0); //Display On, Cursor _ Off, Cursor blink Off
	
	txCmd(0x01); //Clear LCD
}

/** Configuration Functions ***************************************************/
void Fast1602::confEntryMode(bool cursorInc, bool shiftScreen) {
	//Base value
	uint8_t entryMode = 0x04;
	
	//Config
	entryMode |= (cursorInc << 1); //1 = cursor goes right, 0 = goes left
	entryMode |= shiftScreen; //1 = Shift screen instead of cursor
	
	//Write config to screen
	txCmd(entryMode);
}

void Fast1602::confDispMode(bool displayOn, bool cursorOn, bool blinkOn) {
	//Base Value
	uint8_t displayMode = 0x08;
	
	//Config
	displayMode |= (displayOn << 2); //1 = display on, 0 = display off
	displayMode |= (cursorOn << 1); //1 = Cursor _ on, 0 = cursor _ off
	displayMode |= blinkOn; //1 = Cursor Blink on, 0 = cursor blink off

	//Write config to screen
	txCmd(displayMode);
}

void Fast1602::confFunction(bool busLength, bool lineCount, bool fontType) {
	//Base Value
	uint8_t functionSet = 0x20;
	
	//Config
	functionSet |= busLength << 4; //1 = 8Bit, 0 = 4Bit
	functionSet |= lineCount << 3; //1 = 2 line mode, 0 = 1 line mode
	functionSet |= fontType << 2; //1 = 5x11 mode, 0 = 5x8 mode	

	//Write config to screen
	txCmd(functionSet);
}

/** Screen control ************************************************************/
void Fast1602::setPos(uint8_t x, uint8_t y) {	
	uint8_t cmd = 0x80;
	
	//If second line is wanted, convert 0x80 to 0xC0
	if(y == 1) {
		cmd |= 0x40;
	}
	
	//Add the x position to the command
	cmd += x;
	
	//Write that byte out
	txCmd(cmd);
}

void Fast1602::shiftCursor(int8_t c) {
	//Command Variable
	uint8_t cmd;
	
	//If c is positive, set direction to right
	if(c > 0) {
		cmd = 0x14;
	} 
	
	//If c is negative, set direction left and make c positive.
	if(c < 0) {
		cmd = 0x10;
		c = abs(c);
	}
	
	//Max limit on c. 80 chars is the limit from pos 0,0 to the end of display.
	if(c > 80) c = 80;
	
	//Perform the command c number of times
	while(c > 0) {
		txCmd(cmd);
		--c;
	}
}

void Fast1602::shiftScreen(int8_t c) {
	//Command Variable
	uint8_t cmd;
	
	//If c is positive, set direction to right
	if(c > 0) {
		cmd = 0x1C;
	} 
	
	//If c is negative, set direction left and make c positive.
	if(c < 0) {
		cmd = 0x18;
		c = abs(c);
	}
	
	//Max limit on c. 40 chars is one full shift of the display 
	if(c > 40) c = 40;
	
	//Perform the command c number of times
	while(c > 0) {
		txCmd(cmd);
		--c;
	}	
}

/** Text management ***********************************************************/
void Fast1602::printChar(const char chr) {
	txDat(chr);
}

void Fast1602::printString(const char *str) {
	uint8_t length = strlen(str);
	
	//40 chars per line, 80 chars for a full 2 line fill
	if(length > 80) length = 80;
	
	//Print each char from the input string
	for(uint8_t cChar = 0; cChar < length; cChar++) {
		txDat(str[cChar]);
	}
}

void Fast1602::clearChars(uint8_t x, uint8_t y, uint8_t c) {
	//Move to the position
	setPos(x,y);
	
	//Print a space char (0x20) c number of times
	while(c > 0) {
		txDat(0x20);
		--c;
	}
	
	//Move back to the original position
	setPos(x,y);
}

void Fast1602::createCustom(uint8_t index, uint8_t *data) {
	//Move to index in GCRAM. Starts at 0x40 and each char is 8 bytes apart
	txCmd(0x40 + (index * 8));
	
	for(uint8_t cByte = 0; cByte < 8; cByte++) {
		txDat(data[cByte]); //Write 8 bytes to RAM 
	}
	
	//Set DRAM Position. this avoids accidental overwrite of GCRAM due to
	//being in the wrong mode
	txCmd(0x80);
}

void Fast1602::clear() {
	txCmd(0x01);
}

/** Hardware Data Functions ***************************************************/
//Set all dataBus pins to output
void Fast1602::setOutput() {
	for(uint8_t i = 0; i < 4; i++) {
		*hw_DBdir[i] |= hw_DBasm[i]; //OUTPUT
	}
}

//Set all dataBus pind to input
void Fast1602::setInput() {
	for(uint8_t i = 0; i < 4; i++) {
		*hw_DBport[i] &= ~hw_DBasm[i]; //Write low to prevent interference
		*hw_DBdir[i] &= ~hw_DBasm[i]; //Set to INPUT
	}
}

//Writes nibble in lower 4 bits of byte to LCD
void Fast1602::txNibble(uint8_t nibble) {
	//Set write mode
	if(readEn) {
		*hw_RWport &= ~hw_RWasm; //LOW - Write Mode
	}

	for(uint8_t cPin = 0; cPin < 4; cPin++) {
		//Current bit in the byte
		uint8_t bit = (nibble >> cPin) & 0x01; 
		//Write that bit to the correct data pin
		if(bit == 0) {
			*hw_DBport[cPin] &= ~hw_DBasm[cPin]; //Write DB LOW
		} else { 
			*hw_DBport[cPin] |= hw_DBasm[cPin]; //Write DB HIGH
		}
	}
	
	//Cycle enable pin
	*hw_ENport &= ~hw_ENasm; //LOW
	delayMicroseconds(100);
	*hw_ENport |= hw_ENasm; //HIGH
}


//// Non library function to make 2 functions smaller and neater ////
void txWait(char type, uint8_t txByte) {
	//TODO 
	//If read is enable, read busy flag until cmd is complete
	
	//return;
	
	//If reading is not enabled, do manual delays:
	//Most commands take 37us, including writing to DRAM/GRAM.
	//Clear and return commands take 1500us or more.
	
	//TODO can i make this neater and not need a second input?
	if(type == 'c') {
		if(txByte == 0x01 || txByte == 0x02) {
			delay(2);
		}
	} else {
		delayMicroseconds(100);
	}	
}

//Send Instruction/Command to the LCD
void Fast1602::txCmd(uint8_t data) {
	//Set Reg Select
	*hw_RSport &= ~hw_RSasm; //LOW - Instruction
	
	//First nibble is the top 4 bits. (MSBFirst)
	txNibble(data >> 4);
	
	//Second nibble is the lower 4 bits.
	txNibble(data & 0x0F);
	
	//Wait for the command to finish processing 
	txWait('c', data);
}

//Send RAM Data to the LCD
void Fast1602::txDat(uint8_t data) {
	//Set Reg Select
	*hw_RSport |= hw_RSasm; //HIGH - Data
	
	//First nibble is the top 4 bits. (MSBFirst)
	txNibble(data >> 4);
	
	//Second nibble is the lower 4 bits.
	txNibble(data & 0x0F);
	
	//Wait for the command to finish processing
	txWait('d', data);
} 	

