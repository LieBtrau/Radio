/* based on code by Matthias Hertel, http://www.mathertel.de
 */

#include <Arduino.h>
#include <RDA5807M.h>
#include "radiointerfacei2c.h"

RadioInterfaceI2c radi2c;
RDA5807M radio(&radi2c);    ///< Create an instance of a RDA5807 chip radio
unsigned long time;
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
    if(radio.setFrequency(9860))           //102.8 MHz
    {
        Serial.println("tuning ok");
    }
    radio.setVolume(1);
    Serial.println("Radio ok");
    time=millis();
} // setup


/// show the current chip data every 3 seconds.
void loop() {
    if(Serial.available())
    {
        radio.seekUp();
    }
    if(millis()>time+1000)
    {
        radio.checkRDS();
        time=millis();
        String strFreq = String(radio.getFrequency());
        String strOutput = "Frequency :"+ strFreq.substring(0, strFreq.length()-2) \
                + "." + strFreq.substring(strFreq.length()-2)+"MHz";
        Serial.println(strOutput);
        Serial.read();
    }
} // loop

// End.

