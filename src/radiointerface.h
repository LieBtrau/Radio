#pragma once

#include "Arduino.h"

class RadioInterface
{
public:
    virtual void init()=0;
    virtual bool isDetected(byte address)=0;
    virtual bool send(byte address, byte* data, byte length)=0;
    virtual bool sendReceive(byte address, byte* wData, byte wLength, byte* rData, byte rLength)=0;
    virtual bool receive(byte address, byte* data, byte length)=0;
};
