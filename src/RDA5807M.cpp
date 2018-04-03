/* Based on :
 *  Hardware: Arduino Nano with RDA 5807 Chip
 *  Version:  0.5 2013-12-01
 *  Autor:    R.Hoermann RBS Ulm
 *  Homepage: arduino.vom-kuhberg.de =>Projekte/RDA-Radio
 *
 */

#include "RDA5807M.h"

RDA5807M::RDA5807M(RadioInterface* prf): RADIO(prf)
{
    memset(aui_RDA5807_Reg, 0, sizeof(aui_RDA5807_Reg));
}

bool RDA5807M::checkRDS()
{
    if(millis()<rdsPollTime+50)
    {
	return false;
    }
    rdsPollTime=millis();

    if(!_readRegisters(&aui_RDA5807_Reg[0xA]) || !bitRead(aui_RDA5807_Reg[0xA],R0A_RDSS))
    {
        return false;
    }
    if(!bitRead(aui_RDA5807_Reg[0xA],R0A_RDSR) || ((aui_RDA5807_Reg[0xA]&0x3==3)) || ((aui_RDA5807_Reg[0xA]&0xC==0xC)))
    {
        return false;
    }
    _sendRDS(aui_RDA5807_Reg[0xC], aui_RDA5807_Reg[0xD],aui_RDA5807_Reg[0xE],aui_RDA5807_Reg[0xF]);
    return true;
}

RADIO_FREQ RDA5807M::getFrequency(void)
{
    if(!readReg(0xA, aui_RDA5807_Reg[0xA]))
    {
        return 0;
    }
    word channel = aui_RDA5807_Reg[0xA] & 0x03FF;
    return _freqLow + _freqSteps * channel;
}

bool RDA5807M::getRadioInfo(RADIO_INFO *info)
{
    RADIO::getRadioInfo(info);

    if(!_readRegisters(&aui_RDA5807_Reg[0xA]))
    {
        return false;
    }
    info->stereo = bitRead(aui_RDA5807_Reg[0xA], R0A_ST);
    info->mono = bitRead(aui_RDA5807_Reg[0x2], R02_MONO);
    info->rds = bitRead(aui_RDA5807_Reg[0xA], R0A_RDSR);
    info->rssi = aui_RDA5807_Reg[0xB]>>9;
    info->tuned = bitRead(aui_RDA5807_Reg[0xB], R0B_FM_TRUE) && bitRead(aui_RDA5807_Reg[0xB], R0B_FM_READY);
    return true;
}

bool RDA5807M::init()
{
    _pRadio->init();
    if(!reset() || !powerOn(true))
    {
        return false;
    }
//    bitSet(aui_RDA5807_Reg[4],R04_DE);//de-emphasis 50µs
//    writeReg(4);
    bitSet(aui_RDA5807_Reg[5],R05_INT_MODE);
    aui_RDA5807_Reg[5] = R05_SEEKTH<<8 | R05_LNA_PORT_SEL<<6 | R05_LNA_ICSEL_BIT<<4;
    setBassBoost(true);
    bitSet(aui_RDA5807_Reg[2], R02_RDS_EN);
    bitSet(aui_RDA5807_Reg[2], R02_NEW_METHOD);
    //bitSet(aui_RDA5807_Reg[2], R02_MONO);
    setBand(RADIO_BAND_FMWORLD);
    setChannelSpacing(KHz50);
    rdsPollTime=millis();
    return setFrequency(_freqLow);
}

bool RDA5807M::powerOn(bool bPowerOn)
{
    if(bPowerOn)
    {
        bitSet(aui_RDA5807_Reg[2], R02_DHIZ);
        bitSet(aui_RDA5807_Reg[2], R02_DMUTE);
    }
    bitWrite(aui_RDA5807_Reg[2], R02_ENABLE, bPowerOn);
    bool bRet=writeReg(2);
    if(!bPowerOn)
    {
        delay(600);
    }
    return bRet;
}

bool RDA5807M::reset()
{
    aui_RDA5807_Reg[2]=0x0000;
    bitSet(aui_RDA5807_Reg[2], R02_SOFT_RESET);
    bool bRet=writeReg(2);
    bitClear(aui_RDA5807_Reg[2], R02_SOFT_RESET);
    delay(50);
    return bRet;
}

void RDA5807M::term()
{

}

void RDA5807M::seekUp(bool toNextSender)
{
    bitSet(aui_RDA5807_Reg[2], R02_SEEKUP);
    bitSet(aui_RDA5807_Reg[2], R02_SEEK);
    writeReg(2);
}

void RDA5807M::seekDown(bool toNextSender)
{
    bitClear(aui_RDA5807_Reg[2], R02_SEEKUP);
    bitSet(aui_RDA5807_Reg[2], R02_SEEK);
    writeReg(2);
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

bool RDA5807M::setBassBoost(bool switchOn)
{
    bitWrite(aui_RDA5807_Reg[2], R02_BASS, switchOn);
    return writeReg(2);
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
    _freq=newF;
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
        readReg(0xA, aui_RDA5807_Reg[0xA]);
        if(bitRead(aui_RDA5807_Reg[0xA], R0A_STC))
        {
            break;
        }
        delay(10);
    };
    writeReg(2);    //Turn RDS on again.
    return bitRead(aui_RDA5807_Reg[0xA], R0A_STC);
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
    return _readRegisters(regs);

}

//------------------------------------------------------------------------------------------------------------------
bool RDA5807M::readReg(byte regNr, word& val)
{
    byte data[2];
    if(!_pRadio->sendReceive(RDA5807_adrr, &regNr, 1, data,2))
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

bool RDA5807M::_saveRegisters()
{
    byte data[10];
    for (byte i=2;i<7;i++)
    {
        registerToArray(aui_RDA5807_Reg[i], &data[(i-2)<<1]);
    }
    return _pRadio->send(RDA5807_adrs, data, sizeof(data));
}

bool RDA5807M::_readRegisters(word* regs)
{
    byte data[12];
    if(!_pRadio->receive(RDA5807_adrs, data, sizeof(data)))
    {
        return false;
    }
    for(byte i=0;i<6;i++)
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
    word data =  (dataIn[0]<<8) | dataIn[1];
    return data;
}
