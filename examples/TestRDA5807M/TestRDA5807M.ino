/* based on code by Matthias Hertel, http://www.mathertel.de
 */

#include <Arduino.h>
#include <RDA5807M.h>
#include "radiointerfacei2c.h"
#include "RDSParser.h"

RadioInterfaceI2c radi2c;
RDA5807M radio(&radi2c);    ///< Create an instance of a RDA5807 chip radio
RDSParser rdsParse;

RADIO_INFO info;

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
    if(radio.setFrequency(9040))           //90.4 MHz
    {
        Serial.println("tuning ok");
    }
    radio.setVolume(1);
    radio.attachReceiveRDS(parseData);
    rdsParse.attachTextCallback(printText);
    rdsParse.attachServicenNameCallback(printServiceName);
    rdsParse.attachTimeCallback(printTime);
    Serial.println("Radio ok");
} // setup


/// show the current chip data every 3 seconds.
void loop() {
    if(Serial.available())
    {
        radio.seekUp();
    }
    Serial.read();
    radio.checkRDS();
    radio.getRadioInfo(&info);
    delay(50);
} // loop

void parseData(uint16_t b1, uint16_t b2,uint16_t b3, uint16_t b4)
{
    rdsParse.processData(b1, b2, b3, b4);
}

void printText(const char* text)
{
    Serial.println(text);
    Serial.println();
}

void printServiceName(const char* text)
{
    Serial.println(text);
    Serial.println();
}

void printTime(unsigned long utcSeconds, char halfHoursOffset)
{
    char buf[50];
    sprintf(buf,"%d, %d\n",utcSeconds, halfHoursOffset);
    Serial.println(buf);
}

// End.

