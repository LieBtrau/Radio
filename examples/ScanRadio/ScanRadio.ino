/// \file ScanRadio.ino 
/// \brief This sketch implements a scanner that lists all availabe radio stations
/// 
/// \author Christoph Tack
/// Base on code from : Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx

#include <RDA5807M.h>
#include "radiointerfacei2c.h"

RadioInterfaceI2c radi2c;
RDA5807M radio(&radi2c);    ///< Create an instance of a RDA5807 chip radio

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
    Serial.println("Radio ok");
    radio.seekUp();
} // setup


void loop()
{
    RADIO_INFO info;

    delay(50);
    if(radio.getRadioInfo(&info) && info.tuned)
    {
        delay(500);
        radio.getRadioInfo(&info);
        String strFreq = String(radio.getFrequency());
        String strOutput = "Frequency :"+ strFreq.substring(0, strFreq.length()-2) \
                + "." + strFreq.substring(strFreq.length()-2)+"MHz RSSI:"+ info.rssi;
        Serial.println(strOutput);
        radio.seekUp();
    }
} // loop

