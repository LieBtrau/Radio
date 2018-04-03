///
/// \file RDSParser.cpp
/// \brief RDS Parser class implementation.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// \details
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// ChangeLog see RDSParser.h.

#include "RDSParser.h"

#define DEBUG_FUNC0(fn)          { Serial.print(fn); Serial.println("()"); }

/// Setup the RDS object and initialize private variables to 0.
RDSParser::RDSParser() {
    memset(this, 0, sizeof(RDSParser));
    psName=_PSName1;
    pRdsText=rdsText1;
} // RDSParser()


void RDSParser::init() {
    strcpy(_PSName1, "--------");
    strcpy(_PSName2, _PSName1);
    strcpy(programServiceName, "        ");
    memset(rdsText1, 0, sizeof(rdsText1));
    _lastTextIDX = 0;
} // init()


void RDSParser::attachServicenNameCallback(receiveServicenNameFunction newFunction)
{
    _sendServiceName = newFunction;
} // attachServicenNameCallback

void RDSParser::attachTextCallback(receiveTextFunction newFunction)
{
    _sendText = newFunction;
} // attachTextCallback


void RDSParser::attachTimeCallback(receiveTimeFunction newFunction)
{
    _sendTime = newFunction;
} // attachTimeCallback


void RDSParser::processData(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4)
{
    // DEBUG_FUNC0("process");
    byte  idx, mins, hours;
    char halfHoursOffset;
    unsigned long modJulDayCode=0;
    unsigned long utcSeconds=0;

    // Serial.print('('); Serial.print(block1, HEX); Serial.print(' '); Serial.print(block2, HEX); Serial.print(' '); Serial.print(block3, HEX); Serial.print(' '); Serial.println(block4, HEX);

    if (block1 == 0) {
        // reset all the RDS info.
        init();
        // Send out empty data
        if (_sendServiceName) _sendServiceName(programServiceName);
        if (_sendText)        _sendText("");
        return;
    } // if

    // analyzing Block 1 = Program Identification Code
    countryCode=block1>>12;
    progAreaCoverage=lowByte(block1>>8);
    progRefNr=lowByte(block1);

    // analyzing Block 2
    gtype = block2 >> 12;
    b0 = bitRead(block2, 11);
    tp = bitRead(block2, 10);
    pty = (block2>>5) & 0x1F;
    app = block2 & 0x1F;

    if(!b0)
    {
        //A-version
        switch(gtype)
        {
        case 0x0:
            //The 0A group allow to transmit basic data (TA, M/S, DI, C1/C0) as well as Alternatives Frequency (AF) and the name of radio (PS Name)
            // The data received is part of the Service Station Name
            idx = app & 0x0003;

            // new data is 2 chars from block 4
            psName[idx<<1]=highByte(block4);
            psName[(idx<<1)+1]=lowByte(block4);

            if(idx==3)
            {
                if(!strncmp(_PSName1, _PSName2, sizeof(_PSName1)))
                {
                    strcpy(programServiceName, _PSName1);
                    if (_sendServiceName)
                    {
                        _sendServiceName(programServiceName);
                    }
                }
                psName = psName==_PSName1 ? _PSName2 : _PSName1;
                memset(psName, 0, sizeof(_PSName1));
            }
            break;
        case 0x2:
            //This 2A group allows to transmit data of radiotext, with a maximum of 64 characters
            _textAB = bitRead(app,4);
            if (_textAB != _last_textAB) {
                _last_textAB = _textAB;
                memset(pRdsText, 0, sizeof(rdsText1));
            }
            idx = app & 0x000F;
            pRdsText[idx<<2]=highByte(block3);
            pRdsText[(idx<<2)+1]=lowByte(block3);
            pRdsText[(idx<<2)+2]=highByte(block4);
            pRdsText[(idx<<2)+3]=lowByte(block4);
            if (!idx)
            {
                if( ((strlen(rdsText1)==sizeof(rdsText1)-1) || strchr(rdsText1,'\r')) &&
                        (strlen(rdsText1)==strlen(rdsText2)) &&
                        (!strncmp(rdsText1, rdsText2, sizeof(rdsText1))) )
                {
                    char* pc;
                    if(pc=strchr(rdsText1,'\r'))
                    {
                        *(pc+1)='\0';
                    }
                    strncpy(rdsText, rdsText1, sizeof(rdsText1));
                    if (_sendText)
                    {
                        _sendText(rdsText);
                    }
                }
                pRdsText = pRdsText==rdsText1 ? rdsText2 : rdsText1;
            }
            break;
        case 0x3:
            //The group 3A is used to transmit the identification of the applications for free usage (ODA) as well as the groups used for these applications
            break;
        case 0x4:
            //The transmitted clock-time and date shall be accurately set to UTC plus local offset time. Otherwise the
            //transmitted CT codes shall all be set to zero
            modJulDayCode = ((block2 & 0x3) << 15) | ((block3 & 0xFFFE) >> 1);
            mins = (block4 >> 6) & 0x3F;
            hours =  ((block3 & 0x0001) << 4) | ((block4 >> 12) & 0x0F);
            //Conversion to UTC from https://en.wikipedia.org/wiki/Julian_day
            utcSeconds= (modJulDayCode - 40587) * 86400 + hours * 3600 + mins * 60;
            halfHoursOffset = !bitRead(block4, 5) ? block4 & 0x1F : -(block4 & 0x1F);
            if(_sendTime)
            {
                _sendTime(utcSeconds, halfHoursOffset);
            } // if
            break;
        case 0x8:
            //The 8A group can be used for Open Data Architecture (ODA) or for Traffic Message Chanel (TMC)
            break;
        case 0xE:
            //The 14A group is used to send EON (Enhanced Other Network) information
            break;
        default:
            //Serial.println(gtype);
            break;
        }
    }
    else
    {
        //B-version
        //Serial.println(gtype);
    }
} // processData()

// End.
