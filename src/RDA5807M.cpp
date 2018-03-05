/* Based on :
 *  Hardware: Arduino Nano with RDA 5807 Chip
 *  Version:  0.5 2013-12-01
 *  Autor:    R.Hoermann RBS Ulm
 *  Homepage: arduino.vom-kuhberg.de =>Projekte/RDA-Radio
 *
 */

#include "RDA5807M.h"

void RDA5807M::checkRDS()
{

}


RADIO_FREQ RDA5807M::getFrequency(void)
{
    return 0;
}

void RDA5807M::getRadioInfo(RADIO_INFO *info)
{
    RADIO::getRadioInfo(info);

    if(!readAllRegs(&aui_RDA5807_Reg[0xA]))
    {
        return;
    }
    info->stereo = bitRead(aui_RDA5807_Reg[0xA], R0A_ST);
    info->mono = bitRead(aui_RDA5807_Reg[0x2], R02_MONO);
    info->rds = bitRead(aui_RDA5807_Reg[0xA], R0A_RDSR);
    info->rssi = aui_RDA5807_Reg[0xB]>>9;
    info->tuned = bitRead(aui_RDA5807_Reg[0xB], R0B_FM_TRUE);
}

bool RDA5807M::init()
{
    _pRadio->init();
    if(!reset() || !powerOn())
    {
        return false;
    }
    setBand(RADIO_BAND_FMWORLD);
    setChannelSpacing(KHz100);
    return setFrequency(_freqLow);
}

bool RDA5807M::powerOn()
{
    bitSet(aui_RDA5807_Reg[3], R03_TUNE);
    bitSet(aui_RDA5807_Reg[2], R02_ENABLE);
    bool bRet=writeAllRegs();
    bitClear(aui_RDA5807_Reg[3], R03_TUNE);
    return bRet;
}

bool RDA5807M::reset()
{
    word aui_RDA5807_Regdef[10] =
    {
        0x0758,  // 00 defaultid
        0x0000,  // 01 not used
        0xD009,  // 02 DHIZ,DMUTE,BASS, POWERUPENABLE,RDS
        0x0000,  // 03
        0x1400,  // 04 DE ? SOFTMUTE
        0x84DF,  // 05 INT_MODE,SEEKTH=0110,????, Volume=15
        0x4000,  // 06 OPENMODE=01
        0x0000,  // 07 unused ?
        0x0000,  // 08 unused ?
        0x0000   // 09 unused ?
    };
    for(byte i=0;i<7;i++)
    {
        aui_RDA5807_Reg[i]=aui_RDA5807_Regdef[i];
    }
    bitSet(aui_RDA5807_Reg[2], R02_SOFT_RESET);
    bitSet(aui_RDA5807_Reg[2], R02_DHIZ);
    bitSet(aui_RDA5807_Reg[2], R02_DMUTE);
    bitSet(aui_RDA5807_Reg[2], R02_BASS);
    bitSet(aui_RDA5807_Reg[2], R02_RDS_EN);
    bitSet(aui_RDA5807_Reg[2], R02_ENABLE);
    bool bRet=writeAllRegs();
    bitClear(aui_RDA5807_Reg[2], R02_SOFT_RESET);
    return bRet;
}

void RDA5807M::term()
{

}

void RDA5807M::seekUp(bool toNextSender)
{

}

void RDA5807M::seekDown(bool toNextSender)
{

}

void RDA5807M::setBand(RADIO_BAND newBand)
{
    RADIO::setBand(newBand);
    switch(_band)
    {
    case RADIO_BAND_FMWORLD:
        bitWrite(aui_RDA5807_Reg[3],R03_BAND1, bitRead(WW,1));
        bitWrite(aui_RDA5807_Reg[3],R03_BAND0, bitRead(WW,0));
        break;
    default:
        return;
    }
}

void RDA5807M::setBassBoost(bool switchOn)
{

}

void RDA5807M::setChannelSpacing(SPACINGS sp)
{
    bitWrite(aui_RDA5807_Reg[3],R03_SPACE1, bitRead(sp,1));
    bitWrite(aui_RDA5807_Reg[3],R03_SPACE0, bitRead(sp,0));
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
}

bool RDA5807M::setFrequency(RADIO_FREQ newF)
{
    word channel;
    if(newF>_freqHigh)
    {
        return false;
    }
    newF-=_freqLow;
    channel= newF / _freqSteps;
    aui_RDA5807_Reg[3]&=0x003F;
    aui_RDA5807_Reg[3]|=channel<<6;
    bitSet(aui_RDA5807_Reg[3], R03_TUNE);
    if(!writeReg(3))
    {
        return false;
    }
    delay(50);
    for(byte i=0;i<100;i++)
    {
        readReg(14, aui_RDA5807_Reg[14]);
        if(bitRead(aui_RDA5807_Reg[14], R0A_STC))
        {
            break;
        }
        delay(10);
    };
    return bitRead(aui_RDA5807_Reg[14], R0A_STC);
}

void RDA5807M::setMono(bool switchOn)
{

}

void RDA5807M::setMute(bool switchOn)
{

}

void RDA5807M::setSoftMute(bool switchOn)
{

}

void RDA5807M::setVolume(byte newVolume)
{
    RADIO::setVolume(newVolume);
    aui_RDA5807_Reg[5]=(aui_RDA5807_Reg[5] & 0xFFF0)| newVolume;
    writeReg(5);
}

bool RDA5807M::debugStatus(word* regs)
{
    word val;
    readReg(0xA, val);
    Serial.println(val, HEX);
    return readAllRegs(regs);

}

//------------------------------------------------------------------------------------------------------------------
bool RDA5807M::readReg(byte regNr, word& val)
{
    byte data[2];
    if(!_pRadio->sendReceive(RDA5807_adrs, &regNr, 1, data,2))
    {
        return false;
    }
    val = arrayToRegister(data);
}


bool RDA5807M::writeReg(byte regNr)
{
    byte data[3];
    data[0]=regNr;
    registerToArray(aui_RDA5807_Reg[regNr],data+1);
    return _pRadio->send(RDA5807_adrr, data, sizeof(data));
}

bool RDA5807M::writeAllRegs()
{
    byte data[10];
    for (byte i=2;i<7;i++)
    {
        registerToArray(aui_RDA5807_Reg[i], &data[(i-2)<<1]);
    }
    return _pRadio->send(RDA5807_adrs, data, sizeof(data));
}

bool RDA5807M::readAllRegs(word* regs)
{
    byte data[16];
    if(!_pRadio->receive(RDA5807_adrs, data, sizeof(data)))
    {
        return false;
    }
    for(byte i=0;i<8;i++)
    {
        regs[i] = arrayToRegister(&data[i<<1]);
    }
    return true;
}

void RDA5807M::registerToArray(word regIn, byte* dataOut)
{
    //RDA5807 is big endian
    dataOut[0]=highByte(regIn);
    dataOut[1]=lowByte(regIn);
}

word RDA5807M::arrayToRegister(byte* dataIn)
{
    //RDA5807 is big endian
    word data =  dataIn[0]*256 + dataIn[1];
    return data;
}
