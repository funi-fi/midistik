/*

  FUNI.FI MIDISTIK, HARDWARE VERSION 0.3
   _____  .___________  .___  ____________________.___ ____  __. 
  /     \ |   \______ \ |   |/   _____/\__    ___/|   |    |/ _|
 /  \ /  \|   ||    |  \|   |\_____  \   |    |   |   |      <  
/    Y    \   ||    `   \   |/        \  |    |   |   |    |  \ 
\____|__  /___/_______  /___/_______  /  |____|   |___|____|__ \
        \/            \/            \/                        \/
                 
                              ATTINY85 Pins
                              =============
                                 _______
                                |   U   |                                     
   RST (SYNC tbd) <- D5/A0  PB5-|       |- VCC                                
   SD -> *SD-prog -> D3/A3  PB3-| ATTINY|- PB2  D2/A1 <- POTI_RIGHT
   Alt.Sensor(ring)  D4/A2  PB4-|   85  |- PB1  D1    -> MIDI OUT
                            GND-|       |- PB0  D0    -> NEOPIXELS / LED
                                |_______|
                                
          (Notice: Chip installed marker facing down on MIDISTIK v0.2 board)
                                

  * Funi.fi Midi randomiser experimentations by Tuomo Tammenpää (@kimitobo)
    https://github.com/funi-fi

  * ATtiny platform derived from 8BitMixtape by Marc "Dusjagr" Dusseiller & co.
    https://github.com/8BitMixtape
    https://wiki.8bitmixtape.cc/#/Home
    
    8BitMixtape & Hex2Wav Arduino IDE integration:
    https://github.com/8BitMixtape/8Bit-Mixtape-NEO/wiki/4_3-IDE-integration

  * based on TinyAudioBoot and hex2wav by Chris Haberer, Fredrik Olofsson, Budi Prakosa
    https://github.com/ChrisMicro/AttinySound
    
  * SVG-to-Shenzhen Inkscape to Kicad DIY PCB pipeline by Budi Prakosa
    https://github.com/badgeek/svg2shenzhen

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
   
 */

#ifdef __AVR__
#include <avr/power.h>
#endif

#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

// hardware description / pin connections
#define MIDICHAN      0xB0
#define NEOPIXELPIN   0
#define LEDPIN        1
#define MIDIPIN       2
#define POTI          A2
#define BUTTON        A3               // Button in Analog pin, from 8BitMixtape code, where two buttons in same pin
#define NUMPIXELS     1                // Option for several neopixels


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial swSerial(5, MIDIPIN); // RX, TX  // RX not used, has to be defined?, TX = MIDI out

/*

-------------------------- MIDI CC NOTES ------------------------------------

  First Midistik experiemnts have been tested with Korg Volca series, but you
  can just check MIDI implementation chart of your MIDI device and change the
  channell, CC addresses and values accordingly in the Main loop.

  
*/

int midiCCtab[8] = 
{
  // 44, 44, 44, 44, 44, 44, 45, 50       // Korg Volca Keys: Cutoff, EGint
  // 57, 58, 40, 41, 57, 58, 40, 41       // Korg Volca Beats:
  // 23, 71, 74, 23, 71, 74, 23, 74       // Roland JX-03 Filters:
  70, 71, 72, 73, 74, 75, 76, 77        // Digitakt Filter section
};

// -------------------------- SETUP -------------------------------------

void setup()
{
  pinMode(NEOPIXELPIN, OUTPUT);
  pinMode(MIDIPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  swSerial.begin(31250);                                     // Starts Software Serial with MIDI speed
  
  pixels.begin();                                            // This initializes the NeoPixel library.
  pixels.setBrightness(20);
  pixels.setPixelColor(0, pixels.Color(255, 255, 0));        // Neopixel color, use eg for debugging versions
  pixels.show();   
  delay(50);
  
}

// -------------------------- MAIN LOOP -------------------------------------

void loop()
{
      // midiClock();                                               // Sends MIDI Clock, comment out if using internal tempo from Volca
      midiMsg(MIDICHAN, midiCCtab[random(0,7)], random(30,127));     // Channel, randomise CC from table, random scale
      blinke();                                                     // Blink LED indicator
      delay(analogReadScaled());                                    // Loop speed from pot

}
// --------------------------- MIDI / CLOCK / BLINK ------------------------------


uint16_t analogReadScaled()                                  // Scaling for pot values, depends on resitors, needs rework
{
  uint16_t value = analogRead(POTI);
  // if (value > 511) value = 511;
  // return value * 2;
  return value;
}

void midiMsg(uint8_t cmd, uint8_t pitch, uint8_t velocity) {  // Same structure as Note on/off for MIDI CC
  swSerial.write(cmd);    
  swSerial.write(pitch);
  swSerial.write(velocity);
} 

void midiClock(){
  swSerial.write(0xF8);                                      // Send MIDI message for clock pulse = 0xF8 or 248 (24ppqn)
  //delay(5);
}

void blinke(){                                               // Blink indicator LED once
  digitalWrite(LEDPIN, HIGH);
  delay(2);
  digitalWrite(LEDPIN, LOW);
}


// -------------------- NEOPIXEL COLOR ------------------------------------

  void setColorAllPixel(uint32_t color) // Re: Option for several neopizels
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, 0); // off
  }
}

void rainbowCycle(uint8_t wait, uint8_t rounds) {
  uint16_t i, j;

  for (j = 0; j < 256 * rounds; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(5);
  }
}

// Input a value 0 to 255 to get a color value. The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
