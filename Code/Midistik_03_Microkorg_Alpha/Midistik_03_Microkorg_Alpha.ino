/*

  FUNI.FI MIDISTIK, HARDWARE VERSION 0.3
   _____  .___________  .___  ____________________.___ ____  __. 
  /     \ |   \______ \ |   |/   _____/\__    ___/|   |    |/ _|
 /  \ /  \|   ||    |  \|   |\_____  \   |    |   |   |      <  
/    Y    \   ||    `   \   |/        \  |    |   |   |    |  \ 
\____|__  /___/_______  /___/_______  /  |____|   |___|____|__ \
        \/            \/            \/                        \/
                 
                           ATTINY85 Pins on v 0.3
                           ======================
                                 _______
                                |   U   |                                     
   RST (SYNC tbd) <- D5/A0  PB5-|       |- VCC                                
   SD -> *SD-prog -> D3/A3  PB3-| ATTINY|- PB2  D2    -> MIDI OUT
               POTI  D4/A2  PB4-|   85  |- PB1  D1    -> Indicator LED
                            GND-|       |- PB0  D0    -> NEOPIXELS
                                |_______|
                                
          (Notice: Chip installed marker facing down on MIDISTIK v0.3 board)

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

#ifdef __AVR__                        // From 8Bitmixtape code, needed?
#include <avr/power.h>
#endif

#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

// hardware description / pin connections
#define NEOPIXELPIN   0
#define LEDPIN        1
#define MIDIPIN       2
#define POTI          A2
#define BUTTON        A3               // Button in Analog pin, from 8BitMixtape code, where two buttons in same pin
#define NUMPIXELS     1                // Option for several neopixels


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial swSerial(5, MIDIPIN);  // RX, TX  // RX not used, has to be defined?, TX = MIDI out


/*

-------------------------- MIDI CC NOTES ------------------------------------

  First Midistik experiemnts have been tested with Korg Volca series, but you
  can just check MIDI implementation chart of your MIDI device and change the
  channell, CC addresses and values accordingly in the Main loop.
  
MICROKORG MIDI CC parameters
https://media.sweetwater.com/store/media/microkorg_doc_manual.pdf
(Page 56)
...

20 OSC 1 level
21 OSC 2 level
22 Noise level

23 Filter EG: Attack
24 Filter EG: Decay
25 Filter EG: Sustain
26 Filter EG: Release

73 Amp EG: Attack
75 Amp EG: Decay
70 Amp EG: Sustain
72 Amp EG: Release

71  VCF RESONANCE
74  VCF CUTOFF
...
*/

// -------------------------- SETUP ------------------------------------

void setup()
{
  pinMode(NEOPIXELPIN, OUTPUT);
  pinMode(MIDIPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  swSerial.begin(31250);                                     // Starts Software Serial with MIDI speed
  
  pixels.begin();                                            // This initializes the NeoPixel library.
  pixels.setBrightness(20);
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));      // Neopixel color, use eg for debugging versions
  pixels.show();   
  delay(50);
  
}

uint16_t analogReadScaled()                                  // Scaling for pot values, depends on resitors, needs rework
{
  uint16_t value = analogRead(POTI);
  value = map(value, 0, 70, 200, 0);
  return value;
}

// -------------------------- MAIN LOOP  ------------------------------------

void loop()
{

    // cycle MIDI cc values, eg 23-26 (Microkorg Filter envelopes)
    for (uint8_t midiCC = 23; midiCC < 27; midiCC ++) { 
    //midiClock();                                              // Sends MIDI Clock, comment out if using internal tempo from Volca
    midiMsg(0xB0, midiCC, random(0, 127));                    // Randomise midi CC 43-45 on Channel 1 (0xB0)
    blinke();                                                 // Blink LED indicator
    delay(analogReadScaled());                                // Loop speed from pot
  }

}

void midiMsg(uint8_t cmd, uint8_t pitch, uint8_t velocity) {  // Same structure as Note on/off for MIDI CC
  swSerial.write(cmd);    
  swSerial.write(pitch);
  swSerial.write(velocity);
} 

void midiClock(){
  swSerial.write(0xF8);                                      // Send MIDI message for clock pulse = 0xF8 or 248 (24ppqn)
}

void blinke(){                                               // Blink indicator LED once
  digitalWrite(LEDPIN, HIGH);
  delay(5);
  digitalWrite(LEDPIN, LOW);
}



// -------------------------- NEOPIXEL COLORS ------------------------------------_
// Derived from 8Bitmixtape code, could be simplifoed, only one Neopixel as default


void setColorAllPixel(uint32_t color) 
{
  uint8_t n;                                                // Option for several neopixels
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, 0); // off
  }
}

void rainbowCycle(uint8_t wait, uint8_t rounds) {
  uint16_t i, j;

  for (j = 0; j < 256 * rounds; j++) {                    // Cycles all colors on wheel
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
