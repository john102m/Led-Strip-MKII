/*
LedWrite.cpp - Library for sending bursts of 24bits to a ESP8266- 01
to RGB LED 5050 strips of length 5M = 50 groups of 3 LEDs
point to note:  the LED strip likes BRG bits but I talk in RGB bits
Created By John McKinney December 14 2020
Released into the public domain
*/

#include "LedWrite.h"

//some information

LedWrite::LedWrite(int pin)
{
    _pin = pin;
    angle = 0;
}

/*
* @param BRG data
*
*      24 bits of BRG   eg 0xFA0BE    MASK MSB then shift 24 bits << left etc etc
 */

void LedWrite::sendData(RGB data)
{
    sendBRGdata(getBRG(data));
    
}

/*
@param RGB struct of bytes and return 24 BRG bits because the LED strip like BRG in that order
*/
int LedWrite::getBRG(RGB led)
{
    int BRG = led.blue;
    BRG = BRG << 8;
    BRG += led.red;
    BRG = BRG << 8;
    BRG += led.green;
    return BRG;
}

//******************************************************************************************************************************************************************
void LedWrite::fillRandom()
{
    RGB randColors[8];
    int randNum = random(2, 7);               // pick 2 to 7  random primary colours;
    for (int i = 0; i < randNum; i++)
    {
        randColors[i] = color[random(7)];       // colour 7 is white, so up to but not including white.
    }

    int count = 0;                           // iterate through colours, filling the LED array
    for (int i = 0; i < 50; i++)
    {
        led[i] = randColors[count++];
        if (count >= randNum) count = 0;
    }
}

//******************************************************************************************************************************************************************
void LedWrite::setColor(RGB col)
{
    for (int i = 0; i < 50; i++)    // send color info to 50 leds
    {
        led[i] = col;
    }
}
//******************************************************************************************************************************************************************
void LedWrite::fillAlternate(int Mod, RGB A, RGB B)
{
    for (int i = 0; i < 50; i++)
    {
        if (!(i % Mod)) {
            led[i] = A;
        }
        else {
            led[i] = B;
        }
    }
}

//**************************************************************************************************************************************************************
RGB LedWrite::dimLED(int brightness, RGB led)    // percentage brightness applied to all three RGB components
{
    led.red = brightness * led.red / 100;
    led.green = brightness * led.green / 100;
    led.blue = brightness * led.blue / 100;
    return led;
}

//**************************************************************************************************************************************************************
float LedWrite::triFn(float angle, float phase)    //    returns a float between 0 and 1 dependent on angle and phase
{
    float rads = angle * pi / 180;
    return abs(cos(rads + phase) + cos(3 * (rads + phase)) * 0.15);    //  add 15% of the 3rd harmonic to approximate a triangle waveform
}

//******************************************************************************************************************************************************************
void LedWrite::wave(int randColor1)
{
    float freq = random(1, 5) * 7.2;                    //  approx 360 divided by 50
    for (int i = 0; i < 50; i++)
    {
        int bright = triFn(freq * i, pi / 2) * 100;
        led[i] = dimLED(bright, color[randColor1]);
        if (bright < 10) randColor1 = random(7);          //  as the triangle wave hits zero? change the colour??
    }
}

//**************************************************************************************************************************************************************
void LedWrite::phasing(RGB col1, RGB col2)                    // every alternate LED brightness phasing
{
    int bright1 = triFn(angle, 0) * 100;                // make an approx triangle with 3rd order harmonic
    int bright2 = triFn(angle, pi / 2) * 100;           // 45 degree  phase shift used for every alternate LED
    //                                                   // so brighness modulation is out of phase with the previous LED
    for (int i = 0; i < 50; i++)
    {
        if (i % 2)
            led[i] = dimLED(bright1, col1);                  // dimLED returns a percentage brightness of the colour supplied
        else
            led[i] = dimLED(bright2, col2);
    }
    angle += 4;                                          //  this funcion updates the angle GLOBAL variable here  *****************
    if (angle > 359) angle -= 360;                       //  the brightness is dependent on the phase angle
}

//******************************************************************************************************************************************************************
void LedWrite::fillRainbow()
{
    int count = 0;                           // iterate through primary colours, filling the LED array
    for (int i = 0; i < 50; i++)
    {
        led[i] = color[count++];
        if (count > 6) count = 0;
    }
}

//******************************************************************************************************************************************************************
void LedWrite::shiftLEDSup() {
    RGB temp = led[49];
    for (int i = 49; i > 0; i--)
    {
        led[i] = led[i - 1];
    }
    led[0] = temp;
}
//******************************************************************************************************************************************************************
void LedWrite::shift(bool dir)
{
    if (dir) shiftLEDSup();
    else shiftLEDSdown();
}
//******************************************************************************************************************************************************************
void LedWrite::shiftLEDSdown() {
    RGB temp = led[0];
    for (int i = 0; i < 49; i++)
    {
        led[i] = led[i + 1];
    }
    led[49] = temp;
}

////////////////////////////////////////
//  global dimflag
//******************************************************************************************************************************************************************
void LedWrite::dimColors()
{    
    for (int i = 0; i < colorCount; i++)
    {
        color[i].red /= 2;
        color[i].green /= 2;
        color[i].blue /= 2;
    }

}
/////////////////////////////////////////////////////
//   global brighten flag
//******************************************************************************************************************************************************************
void LedWrite::brightenColors()
{   
    for (int i = 0; i < colorCount; i++)
    {
        color[i].red *= 2;
        color[i].green *= 2;
        color[i].blue *= 2;
    }
}

////////////////////////////////////////////////////
//   global  LEDchase
//******************************************************************************************************************************************************************
void LedWrite::chaseEffect(int dir, int randColor1)
{
    led[LEDchase] = color[randColor1];
    LEDchase += dir;
    if (LEDchase > 49) LEDchase = 0;
    if (LEDchase < 0)  LEDchase = 49;
}


/*
* 
*  cycle through led data array and send to led strip   adjust data to be 24 bits per LED  and re-arrange RGB to be BRG !!!
*/

////////////////////////////////////////////////
// uses global count
//******************************************************************************************************************************************************************
void LedWrite::swipe(RGB colA, RGB colB)
{
    for (int i = 0; i < 50; i++)
    {
        if (i < count) led[i] = colA;
        else led[i] = colB;
    }
    if (++count > 49)
    {
        count = 0;
        RGB temp;
        temp = colA;
        colA = colB;
        colB = temp;
    }
}

////////////////////////////////////////////////
// uses global count
//******************************************************************************************************************************************************************
void LedWrite::snowfall(byte mod)
{
    for (int i = 0; i < 50; i++)
    {
        led[i] = color[none];
        if (i == (18 - count))
        {
            led[i] = dimLED(80, color[white]);
            led[i - 1] = color[white];
        }
        if (i == (39 + count))
        {
            led[i - 1] = dimLED(80, color[white]);
            led[i] = color[white];
        }
    }
    count += 3;
    if (count > 11) count = 0;
}


//******************************************************************************************************************************************************************
void LedWrite::twinkle(RGB tcol, RGB bcol)     //   modes 12 and 13
{
    //if (randColor1 == randColor2) bcol = color[none];
    int randNum = random(50);

    for (int i = 0; i < 50; i++)
    {
        if ((i >= randNum - spread) && (i <= randNum + spread))
            led[i] = tcol;           // set a random LED a color against a paler background
        else
            led[i] = bcol;
    }
}


void LedWrite::refresh()         
{
    noInterrupts();
    for (int i = 0; i < 50; i++) {

        sendData(led[i]); //0x040F0F  back color white ish

    }
    delayMicroseconds(50);        // LED strip reset pulse;
    interrupts();
    //yield();   //  <<<<<<<<<<-----------------------------------  ADDED 08/12/2020     *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
}
/*
*  @param  col   
*/
void LedWrite::colorChange(RGB col)
{
    backRed += backRadd;
    if (backRed > 90 || backRed < 10) backRadd = -backRadd;         // 10 to 90% red    ********   GLOBAL VARIABLES!!!!!!!!!  **************

    backGreen += backGadd;
    if (backGreen > 80 || backGreen < 10) backGadd = -backGadd;         // 10 to 80% green

    backBlue += backBadd;
    if (backBlue > 60 || backBlue < 10) backBadd = -backBadd;         // 10 to 60% blue

    for (uint8_t i = 0; i < 50; i++)
    {
        led[i].red = (backRed * col.red) / 100;                   // each LED will have a percentage of primary colour
        led[i].green = (backGreen * col.green) / 100;
        led[i].blue = (backBlue * col.blue) / 100;
    }
}
/*
* @param RGBdata
* 
*      24 bits   eg 0xFA0BE    MASK MSB then shift 24 bits << left etc etc
 */
void LedWrite::sendBRGdata(unsigned int RGBdata)
{
    for (int i = 0; i < 24; i++)
    {
        if (0x800000 & RGBdata) {
            digitalWrite(_pin, HIGH);   // 2us  ??            LOGIC ONE   2us  0.5us
            for (int c = 0; c < LONG_TICKS; c++) NOP;// longPulseTicks
            digitalWrite(_pin, LOW);    //  0.5 us ??
            //for (int c = 0; c < shortPulseTicks; c++) NOP;
        }
        else {
            digitalWrite(_pin, HIGH);   //   ??            LOGIC ZERO    0.5us  2us
            // tweaking using the logic analyzer    -->>   2us and 0.5us as near to as possible
            NOP; NOP; NOP; NOP; NOP; NOP;
            //for (int c = 0; c < HIshortPulseTicks; c++) NOP;
            digitalWrite(_pin, LOW);    //   ??
            for (int c = 0; c < LONG_TICKS_LO; c++) NOP;  //LOWlongPulseTicks
        }

        RGBdata = RGBdata << 1;
    }
}
