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

uint8_t playMode = 1;                   // Toggle modes 1-n

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial swSerial(5, MIDIPIN);  // RX, TX  // RX not used, has to be defined?, TX = MIDI out


/*

-------------------------- MIDI CC NOTES ------------------------------------

  First Midistik experiemnts have been tested with Korg Volca series, but you
  can just check MIDI implementation chart of your MIDI device and change the
  channell, CC addresses and values accordingly in the Main loop.
  

KORG VOLCA SAMPLE MIDI CC PARAMETERS
http://www.korg.com/us/products/dj/volca_sample/

07  Volume
10  Pan
40  Sample start
41  Sample length
42  HiCut
43  Speed
44  Pitch EG Int
45  Pitch EG Attack
46  Pitch EG Decay
47  Amp EG Attack
48  Amp EG Decay
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
  setNeopixel(255,255,0);  
  delay(50);
  
}

uint16_t analogReadScaled()                                  // Scaling for pot values, depends on resitors, needs rework
{
  uint16_t value = analogRead(POTI);
  if (value > 511) value = 511;
  return value * 2;
}

// -------------------------- MAIN LOOP  ------------------------------------

void loop()
{

    
  // cycle MIDI channels 1-10 (Volca Sample), cycle 0xB0-0xB1 if just channel 1 or change to some other cycle logic of your choice
    for (uint8_t midiStatusByte = 0xB0; midiStatusByte < 0xB9; midiStatusByte ++) { 

    //midiClock();                                            // Sends MIDI Clock, comment out if using internal tempo from Volca
    midiMsg(midiStatusByte, 43, random(0, 127));              // eg. MIDI CC 43, Sample Speed Volca Sample, play with random ranges.
    blinke();                                                 // Blink LED indicator
    getButton();
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

void getButton(){                                             // Button in Analog pin
  if (analogRead(BUTTON) < 380){                              // More buttons with different pulldown resistors
    playMode = playMode + 1 && 3;
  }
  return;
}



// -------------------------- NEOPIXEL COLORS ------------------------------------_
// Derived from 8Bitmixtape code, could be simplifoed, only one Neopixel as default

void setNeopixel(uint8_t R, uint8_t G, uint8_t B){
  pixels.setPixelColor(0, pixels.Color(R, G, B));
  pixels.show(); 
}
