///
/// \file RDA5807M.h
/// \brief Library header file for the radio library to control the RDA5807M radio chip.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014-2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
/// 
/// \details
/// This library enables the use of the Radio Chip RDA5807M from http://www.rdamicro.com/ that supports FM radio bands and RDS data.
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 12.05.2014 creation of the RDA5807M library.
/// * 28.06.2014 running simple radio
/// * 08.07.2014 RDS data receive function can be registered.

// multi-Band enabled

// - - - - -
// help from: http://arduino.vom-kuhberg.de/index.php
//   http://projects.qi-hardware.com/index.php/p/qi-kernel/source/tree/144e9c2530f863e32a3538b06c63484401bbe314/drivers/media/radio/radio-rda5807.c


#ifndef RDA5807M_h
#define RDA5807M_h

#include <radio.h>

// ----- library definition -----

/// Library to control the RDA5807M radio chip.
class RDA5807M : public RADIO {
public:
    // ----- RDA5807M specific implementations -----
    const uint8_t MAXVOLUME = 15;   ///< max volume level for radio implementations.
    typedef enum {KHz100=0, KHz200=1, KHz50=2}SPACINGS; //25KHz spacing is nowhere used.

    RDA5807M(RadioInterface* prf): RADIO(prf){}

    bool   init();
    void   term();

    // ----- Audio features -----
    void   setVolume(uint8_t newVolume);
    void   setBassBoost(bool switchOn);
    void   setMono(bool switchOn);
    void   setMute(bool switchOn);
    void   setSoftMute(bool switchOn);    ///< Set the soft mute mode (mute on low signals) on or off.

    // ----- Receiver features -----
    void   setBand(RADIO_BAND newBand);
    bool   setFrequency(RADIO_FREQ newF);
    void   setChannelSpacing(SPACINGS sp);
    RADIO_FREQ getFrequency(void);

    void    seekUp(bool toNextSender = true);   // start seek mode upwards
    void    seekDown(bool toNextSender = true); // start seek mode downwards

    // ----- Supporting RDS for RADIO_BAND_FM and RADIO_BAND_FMWORLD
    void    checkRDS();

    // ----- combined status functions -----
    virtual void getRadioInfo(RADIO_INFO *info); ///< Retrieve some information about the current radio function of the chip.

    // ----- Supporting RDS for RADIO_BAND_FM and RADIO_BAND_FMWORLD

    // ----- debug Helpers send information to Serial port

    void    debugScan();               // Scan all frequencies and report a status
    bool    debugStatus(word* data);             // DebugInfo about actual chip data available

private:
    //Bit definitions
    const byte R02_DHIZ=15;
    const byte R02_DMUTE=14;
    const byte R02_MONO=13;
    const byte R02_BASS=12;
    const byte R02_RDS_EN=3;
    const byte R02_SOFT_RESET=1;
    const byte R02_ENABLE=0;
    const byte R03_TUNE=4;
    const byte R03_BAND1=3;
    const byte R03_BAND0=2;
    const byte R03_SPACE1=1;
    const byte R03_SPACE0=0;
    const byte R0A_RDSR=15;
    const byte R0A_STC=14;
    const byte R0A_ST=10;
    const byte R0B_FM_TRUE=8;

    const word RDA5807_adrs=0x10;       // I2C-Address RDA Chip for sequential  Access
    const word RDA5807_adrr=0x11;       // I2C-Address RDA Chip for random      Access
    const word RDA5807_adrt=0x60;       // I2C-Address RDA Chip for TEA5767like Access

    enum {US_EU=0, JPN=1, WW=2, EEUR=3}bands;
    bool reset();
    bool powerOn();
    bool readAllRegs(word *regs);                       ///< Read regs 0x0A and up.
    bool readReg(byte regNr, word &val);
    bool writeReg(byte regNr);
    bool writeAllRegs();                                ///< Write regs 0x02 and up.
    void registerToArray(word regIn, byte* dataOut);
    word arrayToRegister(byte* dataIn);
    word aui_RDA5807_Reg[16];
};

#endif
