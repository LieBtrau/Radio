///
/// \file SI4703.cpp
/// \brief Implementation for the radio library to control the SI4703 radio chip.
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

#include <SI4703.h>

// initialize the extra variables in SI4703
SI4703::SI4703(RadioInterface* prf, byte resetPin, byte sdioPin):
    RADIO(prf),
    _resetPin(resetPin),
    _sdioPin(sdioPin){}

// initialize all internals.
bool SI4703::init() {
    pinMode(_resetPin, OUTPUT);
    pinMode(_sdioPin, OUTPUT);
    digitalWrite(_sdioPin, LOW);
    digitalWrite(_resetPin, LOW); //Put Si4703 into reset
    delay(1); //Some delays while we allow pins to settle
    digitalWrite(_resetPin, HIGH); //Bring Si4703 out of reset with SDIO set to low and SEN pulled high with on-board resistor
    delay(1); //Allow Si4703 to come out of reset

    _pRadio->init();
    if(!_pRadio->isDetected(SI4703_ADR))
    {
        return false;
    }
    _readRegisters(); //Read the current register set
    registers[TEST1] = 0x8100; //Enable the oscillator, from AN230 page 9, rev 0.61 (works)
    if(!_saveRegisters())
    {
        return false;
    }
    delay(500); //Wait for clock to settle - from AN230 page 9

    registers[POWERCFG]=0;
    bitSet(registers[POWERCFG], DMUTE);
    bitSet(registers[POWERCFG], ENABLE);
    bitClear(registers[POWERCFG], DISABLE);
    if(!_saveRegisters())
    {
        return false;
    }
    delay(110); //Max powerup time, from datasheet page 13

    _readRegisters(); //Read the current register set
    bitSet(registers[SYSCONFIG1], RDS); //Enable RDS
    bitSet(registers[SYSCONFIG1], DE); //de-emphasis 50µs
    bitSet(registers[POWERCFG], RDSM); //RDS in verbose mode
    if(!_saveRegisters())
    {
        return false;
    }

    setBand(RADIO_BAND_FMWORLD);
    setChannelSpacing(KHz100);
    setVolume(1);
    setSeekParams(SK_GOOD_Q_ONLY);
    return true;
}

// switch the power off
void SI4703::term()
{
    //DEBUG_FUNC0("term");
}


// ----- Volume control -----
void SI4703::setVolume(byte newVolume)
{
    RADIO::setVolume(newVolume);
    if(!_readRegisters())
    {
        return;
    }
    registers[SYSCONFIG2] &= 0xFFF0;
    registers[SYSCONFIG2] |= _volume;
    _saveRegisters(); //Update
}


// Mono / Stereo
void SI4703::setMono(bool switchOn)
{
    RADIO::setMono(switchOn);
    if(!_readRegisters())
    {
        return;
    }
    if (switchOn)
    {
        bitSet(registers[POWERCFG], MONO);
    }
    else
    {
        bitClear(registers[POWERCFG], MONO);
    }
    _saveRegisters();
}


/// Switch mute mode.
void SI4703::setMute(bool switchOn)
{
    RADIO::setMute(switchOn);
    if(!_readRegisters())
    {
        return;
    }
    if (switchOn)
    {
        bitClear(registers[POWERCFG],DMUTE);
    }
    else
    {
        bitSet(registers[POWERCFG], DMUTE);
    }
    _saveRegisters();

}


/// Switch soft mute mode.
void SI4703::setSoftMute(bool switchOn)
{
    RADIO::setSoftMute(switchOn);
    if(!_readRegisters())
    {
        return;
    }
    if (switchOn)
    {
        bitClear(registers[POWERCFG],DSMUTE);
    }
    else {
        bitSet(registers[POWERCFG],DSMUTE);
    }
    _saveRegisters();
}


// ----- Band and frequency control methods -----

// tune to new band.
void SI4703::setBand(RADIO_BAND newBand)
{
    RADIO::setBand(newBand);
    if(!_readRegisters())
    {
        return;
    }
    switch(_band)
    {
    case RADIO_BAND_FM:
        bitClear(registers[SYSCONFIG2],BAND1);
        bitClear(registers[SYSCONFIG2],BAND0);
        break;
    case RADIO_BAND_FMWORLD:
        bitClear(registers[SYSCONFIG2],BAND1);
        bitSet(registers[SYSCONFIG2],BAND0);
        break;
    default:
        return;
    }
    _saveRegisters();
}

void SI4703::setChannelSpacing(SPACINGS sp)
{
    if(!_readRegisters())
    {
        return;
    }
    bitWrite(registers[SYSCONFIG2], SPACE1, bitRead(sp,1));
    bitWrite(registers[SYSCONFIG2], SPACE0, bitRead(sp,0));
    switch (sp)
    {
    case KHz50:
        _freqSteps = 5;
        break;
    case KHz100:
        _freqSteps = 10;
        break;
    case KHz200:
        _freqSteps = 20;
        break;
    default:
        break;
    }
    _saveRegisters();
}

/**
* @brief Retrieve the real frequency from the chip after automatic tuning.
* @return RADIO_FREQ the current frequency.
*/
RADIO_FREQ SI4703::getFrequency() {
    if(!_readRegisters())
    {
        return false;
    }
    word channel = registers[READCHAN] & 0x03FF;
    _freq = (channel * _freqSteps) + _freqLow;
    return (_freq);
}

/**
* @brief Change the frequency in the chip.
* @param newF
* @return void
*/
bool SI4703::setFrequency(RADIO_FREQ newF)
{
    RADIO::setFrequency(newF);
    if(!_readRegisters())
    {
        return false;
    }
    int channel = (_freq - _freqLow) / _freqSteps;

    //These steps come from AN230 page 20 rev 0.5
    registers[CHANNEL] &= 0xFC00; //Clear out the channel bits
    registers[CHANNEL] |= channel; //Mask in the new channel
    bitSet(registers[CHANNEL], TUNE); //Set the TUNE bit to start
    _saveRegisters();
    return _waitEnd();
}

// start seek mode upwards
bool SI4703::seekUp(bool toNextSender)
{
    return _seek(true);
}


// start seek mode downwards
bool SI4703::seekDown(bool toNextSender)
{
    return _seek(false);
}


/// Retrieve all the information related to the current radio receiving situation.
bool SI4703::getRadioInfo(RADIO_INFO *info)
{
    RADIO::getRadioInfo(info); // all settings to last current settings

    if(!_readRegisters())
    {
        return false;
    }
    info->active = true; // ???
    if (bitRead(registers[STATUSRSSI], ST)) info->stereo = true;
    info->rssi = registers[STATUSRSSI] & RSSI;
    if (registers[STATUSRSSI] & (RDSS)) info->rds = true;
    info->tuned = _tuned;
    if (bitRead(registers[POWERCFG], MONO)) info->mono = true;
    return true;
}


/// Return current audio settings.
void SI4703::getAudioInfo(AUDIO_INFO *info)
{
    RADIO::getAudioInfo(info);

    _readRegisters();
    if (!bitRead(registers[POWERCFG],DMUTE))
    {
        info->mute = true;
    }
    if (!bitRead(registers[POWERCFG],DSMUTE))
    {
        info->softmute = true;
    }
    info->bassBoost = false; // no bassBoost
    info->volume = registers[SYSCONFIG2] & 0x000F;
}


bool SI4703::checkRDS()
{
    if (!_sendRDS)
    {
        return false;
    }
    if(millis()<rdsPollTime+50)
    {
    return false;
    }
    rdsPollTime=millis();
    _readRegisters();
    if(bitRead(registers[STATUSRSSI], RDSS))
    {
        Serial.println("RDS syncro");
    }
    if(bitRead(registers[STATUSRSSI], RDSR))
    {
        _sendRDS(registers[RDSA], registers[RDSB], registers[RDSC], registers[RDSD]);
    }
}

void SI4703::setSeekParams(SEEK_PARAMS sk)
{
    if(sk >= SK_MAX)
    {
        return;
    }
    _readRegisters();
    registers[SYSCONFIG2]|= (SEEKTH_MIN + seekParams[sk].seekth) & SEEKTH_MASK;
    registers[SYSCONFIG3]|= (SKSNR_MIN + seekParams[sk].sksnr) & SKSNR_MASK;
    registers[SYSCONFIG3]|= (SKCNT_MIN + seekParams[sk].skcnt) & SKCNT_MASK;
    _saveRegisters();
}

// ----- Debug functions -----

/// Send the current values of all registers to the Serial port.
void SI4703::debugStatus()
{
    _readRegisters();
     for (int x = 0 ; x < 16 ; x++)
     {
        Serial.print("Reg: 0x0"); Serial.print(x, HEX);
        Serial.print(" = 0x"); _printHex4(registers[x]);
        Serial.println();
    }
}

bool SI4703::_seek(bool seekUp)
{
    bool tuned;
    _readRegisters();
    if(bitRead(registers[POWERCFG], SEEK))
    {
        bitClear(registers[POWERCFG], SEEK);
        _saveRegisters();
    }
    if(seekUp)
    {
        bitSet(registers[POWERCFG], SEEKUP);
    }
    else
    {
        bitClear(registers[POWERCFG], SEEKUP);
    }
    bitSet(registers[POWERCFG], SKMODE);
    bitSet(registers[POWERCFG], SEEK);
    _saveRegisters();
    return _waitEnd();
}

bool SI4703::_waitEnd()
{
    bool bResult=false;
    _tuned=false;
    delay(50);
    for(byte i=0;i<100;i++)
    {
        if(!_readRegisters())
        {
            return false;
        }
        if(bitRead(registers[STATUSRSSI], STC))
        {
            bResult=true;
            break;
        }
        delay(60);  //Seek/Tune Time (datasheet Table 8.)
    };
    _tuned = bitRead(registers[STATUSRSSI], SFBL)? false : true;
    bitClear(registers[POWERCFG], SEEK);
    bitClear(registers[CHANNEL], TUNE);
    if(!_saveRegisters())
    {
        return false;
    }
    while(bitRead(registers[STATUSRSSI], STC))
    {
        if(!_readRegisters())
        {
            return false;
        }
        delay(10);
    }
    return bResult;
}


// ----- internal functions -----
bool SI4703::_saveRegisters()
{
    byte data[12];
    for (byte i=2;i<=7;i++)
    {
        registerToArray(registers[i], &data[(i-2)<<1]);
    }
    return _pRadio->send(SI4703_ADR, data, sizeof(data));
}

bool SI4703::_readRegisters()
{
    //Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
    //We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
    byte data[32];
    if(!_pRadio->receive(SI4703_ADR, data, sizeof(data)))
    {
        return false;
    }
    for(byte i=0;i<16;i++)
    {
        registers[i] = arrayToRegister(&data[((i+6)%16)<<1]);
    }
    return true;
}

void SI4703::registerToArray(word regIn, byte* dataOut)
{
    //SI4703 is big endian
    dataOut[0]=highByte(regIn);
    dataOut[1]=lowByte(regIn);
}

word SI4703::arrayToRegister(byte* dataIn)
{
    //SI4703 is big endian
    word data =  (dataIn[0]<<8) | dataIn[1];
    return data;
}



