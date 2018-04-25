/// \file ScanRadioWithRds.ino
/// \brief This sketch implements a scanner that lists all availabe radio stations
///
/// \author Christoph Tack
/// Base on code from : Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx

#include <RDA5807M.h>
#include <SI4703.h>
#include "radiointerfacei2c.h"
#include "RDSParser.h"

RadioInterfaceI2c radi2c;
//RDA5807M radio(&radi2c);    ///< Create an instance of a RDA5807 chip radio
//SI4703   radio(&radi2c, 2, 14);//Nucleo
SI4703   radio(&radi2c, 3, A4);//protrinket
RDSParser rdsParse;
unsigned long ulStartTime;
bool bNew=true;

void setup()
{
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
    radio.setVolume(1);
    radio.attachReceiveRDS(parseData);
    rdsParse.attachTextCallback(printText);
    rdsParse.attachServicenNameCallback(printServiceName);
    rdsParse.attachTimeCallback(printTime);
    Serial.println("Radio ok");
    radio.seekUp();

//    radio.setFrequency(9040);
    ulStartTime=millis();
} // setup


void loop()
{
    RADIO_INFO info;

    delay(50);
    radio.checkRDS();
    if(radio.getRadioInfo(&info) && info.tuned && bNew)
    {
        delay(500);
        radio.getRadioInfo(&info);
        String strFreq = String(radio.getFrequency());
        String strOutput = "Frequency :"+ strFreq.substring(0, strFreq.length()-2) \
                + "." + strFreq.substring(strFreq.length()-2)+"MHz RSSI:"+ info.rssi;
        Serial.println(strOutput);
        bNew=false;
    }
    if(millis()>ulStartTime+30000)
    {
        Serial.println("next channel...");
        radio.seekUp();
        ulStartTime=millis();
        bNew=true;
    }
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
