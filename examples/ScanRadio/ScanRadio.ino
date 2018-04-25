/// \file ScanRadio.ino 
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

RadioInterfaceI2c radi2c;
//RDA5807M radio(&radi2c);    ///< Create an instance of a RDA5807 chip radio
#ifdef ARDUINO_AVR_PROTRINKET3FTDI
SI4703 radio(&radi2c, 3, A4);
#elif defined(ARDUINO_STM_NUCLEO_F103RB)
SI4703 radio(&radi2c, D2, 14);
#else
radio(&radi2c, 2, A4);
#endif
void setup()
{
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Radio...");
    delay(200);
    if(!radio.init())
    {
        Serial.println("Can't init radio");
        while(1);
    }
    //radio.setBand(RADIO_BAND_FMWORLD);
    radio.setVolume(1);
    Serial.println("Radio ok");
    radio.seekUp();
    //radio.setFrequency(10210);
} // setup


void loop()
{
    RADIO_INFO info;

    if(!radio.seekUp())
    {
        Serial.println("false");
        return;
    }
    radio.getRadioInfo(&info);
    String strFreq = String(radio.getFrequency());
    String strOutput = "Frequency :"+ strFreq.substring(0, strFreq.length()-2) \
            + "." + strFreq.substring(strFreq.length()-2)+"MHz RSSI:"+ info.rssi;
    Serial.println(strOutput);
    delay(2000);
} // loop

