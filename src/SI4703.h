///
/// \file SI4703.h
/// \brief Library header file for the radio library to control the SI4703 radio chip.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014-2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// This library enables the use of the Radio Chip SI4703.
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 05.08.2014 created.


#pragma once
#include <radio.h>

// ----- library definition -----


/// Library to control the SI4703 radio chip.
class SI4703 : public RADIO {
public:
    typedef enum {KHz200=0, KHz100=1, KHz50=2}SPACINGS; //25KHz spacing is nowhere used.
    typedef enum{SK_DEFAULT, SK_RECOMMENDED, SK_MORE_STATIONS, SK_GOOD_Q_ONLY, SK_MOST_STATIONS, SK_MAX}SEEK_PARAMS;
    typedef struct{
        byte seekth;
        byte sksnr;
        byte skcnt;
    }DEFAULT_SEEK_PARAMS;
    const DEFAULT_SEEK_PARAMS seekParams[5]=
    {
        {0x19,0,0}, {0x19,4,8}, {0xC,4,8}, {0xC, 7, 0xF}, {0,4,0xF}
    };
    const uint8_t MAXVOLUME = 15;   ///< max volume level for radio implementations.

    SI4703(RadioInterface *prf, byte resetPin, byte sdioPin);

    bool   init();  // initialize library and the chip.
    void   term();  // terminate all radio functions.

    // Control of the audio features

    // Control the volume output of the radio chip
    void    setVolume(byte newVolume); ///< Control the volume output of the radio chip in the range 0..15.

    // Control mono/stereo mode of the radio chip
    void   setMono(bool switchOn); // Switch to mono mode.

    // Control the mute function of the radio chip
    void   setMute(bool switchOn); // Switch to mute mode.

    // Control the softMute function of the radio chip
    void   setSoftMute(bool switchOn); // Switch to soft mute mode.
    void setSeekParams(SEEK_PARAMS sk);

    // Control of the core receiver

    // Control the frequency
    void   setBand(RADIO_BAND newBand);
    void   setChannelSpacing(SPACINGS sp);

    bool    setFrequency(RADIO_FREQ newF);
    RADIO_FREQ getFrequency(void);

    bool seekUp(bool toNextSender = true);   // start seek mode upwards
    bool seekDown(bool toNextSender = true); // start seek mode downwards

    bool checkRDS(); // read RDS data from the current station and process when data available.

    // ----- combined status functions -----

    virtual bool getRadioInfo(RADIO_INFO *info); ///< Retrieve some information about the current radio function of the chip.

    virtual void getAudioInfo(AUDIO_INFO *info); ///< Retrieve some information about the current audio function of the chip.

    // ----- debug Helpers send information to Serial port

    void  debugScan();               // Scan all frequencies and report a status
    void  debugStatus();             // Report Info about actual Station

    // ----- read/write registers of the chip

private:
    // ----- local variables
    static const byte SI4703_ADR=0x10;

    //Registers
    // Define the register names
    static const byte DEVICEID = 0x00;
    static const byte CHIPID = 0x01;
    static const byte POWERCFG = 0x02;
    static const byte CHANNEL = 0x03;
    static const byte SYSCONFIG1 = 0x04;
    static const byte SYSCONFIG2 = 0x05;
    static const byte SYSCONFIG3 = 0x06;
    static const byte TEST1 = 7;
    static const byte STATUSRSSI = 0x0A;
    static const byte READCHAN = 0x0B;
    static const byte RDSA = 0x0C;
    static const byte RDSB = 0x0D;
    static const byte RDSC = 0x0E;
    static const byte RDSD = 0x0F;

    //Register 0x02 - POWERCFG
    static const byte DSMUTE = 15;
    static const byte DMUTE = 14;
    static const byte MONO = 13;
    static const byte RDSM = 11;
    static const byte SKMODE = 10;
    static const byte SEEKUP = 9;
    static const byte SEEK = 8;
    static const byte DISABLE = 6;
    static const byte ENABLE = 0;

    //Register 0x03 - CHANNEL
    static const byte TUNE = 15;

    //Register 0x04 - SYSCONFIG1
    static const byte RDS = 12;
    static const byte DE = 11;

    //Register 0x05 - SYSCONFIG2
    static const word SEEKTH_MASK = 0x7F00;
    static const word SEEKTH_MIN = 0x00FF;

    static const byte BAND1 = 7;
    static const byte BAND0 = 6;
    static const byte SPACE1 = 5;
    static const byte SPACE0 = 4;

    // Register 0x06 - SYSCONFIG3
    static const word SKSNR_MASK = 0x0070;
    static const word SKSNR_MIN  = 0x000F;

    static const word SKCNT_MASK = 0x000F;
    static const word SKCNT_MIN  = 0x0000;

    //Register 0x0A - STATUSRSSI
    static const word RDSR   = 15; ///<RDS ready
    static const byte STC    = 14; ///<Seek Tune Complete
    static const byte SFBL   = 13; ///< Seek Fail Band Limit
    static const word AFCRL  = 0x1000;
    static const word RDSS   = 0x0800; ///<RDS syncronized
    static const word ST     = 8; ///< Stereo Indicator
    static const byte RSSI   = 0x00FF;

    // store the current values of the 16 chip internal 16-bit registers
    uint16_t registers[16];

    // ----- low level communication to the chip using I2C bus
    bool  _readRegisters();  // read all status & data registers
    bool  _saveRegisters();  // Save writable registers back to the chip

    void registerToArray(word regIn, byte* dataOut);
    word arrayToRegister(byte* dataIn);

    bool _seek(bool seekUp = true);
    bool _waitEnd();
    byte _resetPin;
    byte _sdioPin;
    bool _tuned=false;
};
