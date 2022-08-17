/* This is a faster Library for the LCD1602 using assembly instead of 
 * digitalwrite for higher speeds.
 *
 * Features ------
 * Optionally select if reading is enabled. Saves a pin on the host device.
 * Incredibly fast execution for better refresh speeds and less flicker.
 * More features than the previous library - including selective blanking,
 *   masking, and clearing functions
 *
 *
 *
 * Version 0.50 -- Needs read and optimisations
 * ADBeta 13 Aug 2022
*/

#include <Arduino.h>

#ifndef FAST1602_H
#define FAST1602_H

class Fast1602 {
	public:
	/** Setup and Init Functions **********************************************/
	/*-------------------------------------------------------------------------- 
		One Constructor allows reading+writing, the other only allows writing.
		This saves a pin on the host if reading is not desired or required.
	--------------------------------------------------------------------------*/
	//RS, RW, EN, DB Array -- 4 pin setup - D4, D5, D6, D7
	Fast1602(const uint8_t, const uint8_t, const uint8_t, const uint8_t*);
	//RS, EN, DB Array -- 4 pin setup - D4, D5, D6, D7
	Fast1602(const uint8_t, const uint8_t, const uint8_t*);
	
	/** LCD HardwareConfiguration Functions **/	
	//TODO change this all up and be verbose 
	void confEntryMode(bool, bool);
	void confDispMode(bool, bool, bool);
	void confFunction(bool, bool, bool);
	
	//Initialise the LCD with the chosen config and prepare it for use
	void init();
	
	/** Text management *******************************************************/
	//Print a single char to the screen (uses the 1602A inbuilt font index)
	void printChar(const char);
	//Print a string of chars to the LCD. //TODO wrapping?
	void printString(const char*);
	//Clears a chunk of chars from x,y position, c number of chars.
	void clearChars(uint8_t x, uint8_t y, uint8_t c);
	//TODO an update chunk function that knows the pos of a UI element and 
	//manages the clearing and updating (?)
	//Clear the entire screen and return the cursor to 0,0
	void clear();
	
	/** Screen Control ********************************************************/
	//Set the position of the GRAM cursor, x y, starting at 0
	void setPos(uint8_t x, uint8_t y);
	//Shift the cursor by an amount. negative numbers move left.
	void shiftCursor(int8_t);
	//Shift the screen x number of times. nagative numbers move left.
	void shiftScreen(int8_t);
	//Create a custom character, 8 byte array, 5 bits per byte, LSB(?)
	void createCustom(uint8_t index, uint8_t* data);
	
	private:
	//Read enable flag.
	bool readEn;
	
	/** Pins, ports and register **********************************************/
	/* Assemly pins */
	uint8_t hw_RSasm, hw_RWasm, hw_ENasm;
	uint8_t hw_DBasm[4];
	/* Assembly ports */
	volatile uint8_t *hw_RSport, *hw_RWport, *hw_ENport;
	volatile uint8_t *hw_DBport[4];
	/* Data Direction Register, and readPort (DataBus only) */
	volatile uint8_t *hw_DBdir[4]; 
	volatile uint8_t *hw_DBread[4];
	
	/** Hardware Data Functions ***********************************************/
	//Manage dual nibble byte transceive
	void txNibble(uint8_t); //Write a 4 bit nibble to the bus (LSBFirst)
	//TODO receive nibble
	//Send byte of command or data
	void txCmd(unsigned char); //Write instruction to the LCD
	void txDat(unsigned char); //Write data to LCD
	//Input output control of the dat lines
	void setOutput(); //Sets the DB pins to output
	void setInput(); //Sets the DB pins to input
}; //class Fast1602

#endif
