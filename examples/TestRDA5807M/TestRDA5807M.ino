/* based on code by Matthias Hertel, http://www.mathertel.de
 */

#include <Arduino.h>
#include <RDA5807M.h>
#include "radiointerfacei2c.h"

RadioInterfaceI2c radi2c;
RDA5807M radio(&radi2c);    ///< Create an instance of a RDA5807 chip radio

void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Radio...");
    delay(200);
    if(!radio.init())
    {
        Serial.println("Can't init radio");
        return;
    }
    radio.setBand(RADIO_BAND_FMWORLD);
    if(radio.setFrequency(10210))           //102.1 MHz
    {
        Serial.println("tuning ok");
    }
    radio.setVolume(1);
    Serial.println("Radio ok");
} // setup


/// show the current chip data every 3 seconds.
void loop() {
    delay(3000);
    word data[8];
    char buffer [50];
    if(!radio.debugStatus(data))
    {
        Serial.println("no data");
        return;
    }
    for(byte i=0;i<8;i++)
    {
        sprintf (buffer, "0x%02X, 0x%04X", i+0xa, data[i]);
        Serial.println(buffer);
    }
} // loop

// End.

