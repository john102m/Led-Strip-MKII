/*
LedWrite.h - Library for sending bursts of 24bits 
to RGB LED 5050 strips of length 5M = 50 groups of 3 LEDs
from an ESP 8266-01 connected to the WiFI
Created By John McKinney December 14 2020
Released into the public domain
*/
#ifndef LedWrite_h
#define LedWrite_h

#include "Arduino.h"

#define NOP __asm__ __volatile__ ("nop\n\t")

struct RGB {
	byte red;
	byte green;
	byte blue;
};

enum COLOR { red, orange, yellow, green, blue, indigo, violet, white, lemon, back, none, bright };

class LedWrite
{
	public:

		int spread = 1;
		LedWrite(int pin);
		RGB* getColor(){ return this->color; }
		void refresh();
		void colorChange(RGB col);
		void wave(int randColor1);
		void shift(bool dir);
		void setColor(RGB col);
		void fillRainbow();
		void fillAlternate(int Mod, RGB A, RGB B);		
		void phasing(RGB col1, RGB col2);
		void fillRandom();
		RGB dimLED(int brightness, RGB led);
		void dimColors();
		void brightenColors();
		void chaseEffect(int dir, int col);
		void twinkle(RGB tcol, RGB bcol);
		void snowfall(byte mod);
		void swipe(RGB colA, RGB colB);
    static const int colorCount = 12;


	private:
		// Tested on ESP8266-01 @ 160MHz 
		// Arduino Version 1.8.xx
		// const int longPulseTicks = 24; const int LOWlongPulseTicks = 20;
		// int shortPulseTicks = 2;
		// int HIshortPulseTicks = 1;

		const int LONG_TICKS = 24;
		const int LONG_TICKS_LO = 20;
		const float pi = 3.141592653589;

		//brightness adjustment
		int RbrightAdj = 5, GbrightAdj = 7, BbrightAdj = 5;    // some GLOBAL variables for LED dimming
		// background colour component
		int backRed = 10, backGreen = 10, backBlue = 10;
		//initial amount to add to the back ground colour component
		int backRadd = 5, backGadd = 5, backBadd = 5;

		// the pin on which to output data on the ESP8266-01
		int _pin;
		RGB led[50];
		RGB color[colorCount] = 
		{
			{0x80, 0x00, 0x00},   // red
			{0xC4, 0x1A, 0x00},   // orange
			{0x5F, 0x27, 0x00},   // yellow
			{0x00, 0x80, 0x00},   // green
			{0x00, 0x00, 0x80},   // blue
			{0x80, 0x00, 0x80},   // indigo
			{0xC4, 0x00, 0x7D},   // violet
			{0x80, 0x80, 0x80},   // white
			{0x0B, 0x03, 0x00},   // lemon
			{0x04, 0x0F, 0x0F},   // back                      //  e.g   led[6] = color[indigo];
			{0x00, 0x00, 0x00},   // none
			{0xFF, 0xFF, 0xFF}    // bright  (white)
		};
		void sendData(RGB data);
    void sendBRGdata(unsigned int RGBdata);
    int getBRG(RGB led);

  	void shiftLEDSup();
  	void shiftLEDSdown();
  	float triFn(float angle, float phase);
  	int LEDchase = 1;

  	//count is used internally used by swipe and snowfall (gets incremented every time they are invoked)
  	int count = 0;
  	// used by phasing method to calculate a brightness from a triangle wave whos phase is shifted by this angle
  	int angle = 0;

};
#endif
