/// \file Radio.cpp
/// \brief Library implementation for the radio libraries to control radio chips.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// \details
/// This library enables the use of diverse radio chips by sharing the same class definition.
/// Implementation for the following Radio Chips are available:
/// * RDA5807M
/// * SI4703
///
/// The following chip is planned to be supported too:
/// * TEA5767
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// ChangeLog see: radio.h 

#include "Arduino.h"

#include "radio.h"

// ----- Register Definitions -----

// no chip-registers without a chip.


// ----- implement


/// The RADIO class doesn't implement a concrete chip so nothing has to be initialized.
bool RADIO::init() {
    return(false);
} // init()


/// switch the power off
/// The RADIO class doesn't implement a concrete chip so nothing has to be terminated.
void RADIO::term() {
} // term()


// ----- Volume control -----

void RADIO::setVolume(byte newVolume) {

    _volume = newVolume > MAXVOLUME ? MAXVOLUME : newVolume;
} // setVolume()


uint8_t RADIO::getVolume() {
    return(_volume);
} // getVolume()


// ----- bass boost control -----

/// Control the bass boost mode of the radio chip.
/// The base implementation ony stores the value to the internal variable.
/// @param switchOn true to switch bassBoost mode on, false to switch bassBoost mode off.
bool RADIO::setBassBoost(bool switchOn) {
    _bassBoost = switchOn;
} // setBassBoost()


/// Retrieve the current bass boost mode setting.
/// The base implementation returns only the value in the internal variable.
bool RADIO::getBassBoost() {
    return(_bassBoost);
} // getBassBoost()


// ----- mono control -----

/// The base implementation ony stores the value to the internal variable.
void RADIO::setMono(bool switchOn) {
    _mono = switchOn;
} // setMono()


/// The base implementation returns only the value in the internal variable.
bool RADIO::getMono() {
    return(_mono);
} // getMono()


// ----- mute control -----

/// The base implementation ony stores the value to the internal variable.
void RADIO::setMute(bool switchOn) {
    _mute = switchOn;
} // setMute()


/// The base implementation returns only the value in the internal variable.
bool RADIO::getMute() {
    return(_mute);
} // getMute()


// ----- softmute control -----

/// The base implementation ony stores the value to the internal variable.
void RADIO::setSoftMute(bool switchOn) {
    _softMute = switchOn;
} // setSoftMute()


/// The base implementation returns only the value in the internal variable.
bool RADIO::getSoftMute() {
    return(_softMute);
} // getSoftMute()


// ----- receiver control -----

// some implementations to return internal variables if used by concrete chip implementations

/// Start using the new band for receiving.
void RADIO::setBand(RADIO_BAND newBand) {
    _band = newBand;
    if (newBand == RADIO_BAND_FM) {
        _freqLow = 8700;
        _freqHigh = 10800;
    }
    else if (newBand == RADIO_BAND_FMWORLD) {
        _freqLow = 7600;
        _freqHigh = 10800;
    } // if
} // setBand()


/// Start using the new frequency for receiving.
/// The new frequency is stored for later retrieval.
bool RADIO::setFrequency(RADIO_FREQ newFreq) {
    if (newFreq < _freqLow)  newFreq = _freqLow;
    if (newFreq > _freqHigh) newFreq = _freqHigh;
    _freq = newFreq;
} // setFrequency()


void RADIO::setBandFrequency(RADIO_BAND newBand, RADIO_FREQ newFreq) {
    setBand(newBand);
    setFrequency(newFreq);
} // setBandFrequency()


bool RADIO::seekUp(bool)   {}
bool RADIO::seekDown(bool) {}

RADIO_BAND RADIO::getBand()         { return(_band); }
RADIO_FREQ RADIO::getFrequency()    { return(_freq); }
RADIO_FREQ RADIO::getMinFrequency() { return(_freqLow); }
RADIO_FREQ RADIO::getMaxFrequency() { return(_freqHigh); }
RADIO_FREQ RADIO::getFrequencyStep(){ return(_freqSteps); }


/// Return all the Radio settings.
/// This implementation only knows some values from the last settings.
bool RADIO::getRadioInfo(RADIO_INFO *info) {
    // set everything to false and 0.
    memset(info, 0, sizeof(RADIO_INFO));
    // info->tuned = false;
    // info->rds = false;
    // info->stereo = false;

    // use current settings
    info->mono = _mono;

} // getRadioInfo()


/// Return current settings as far as no chip is required.
/// When using the radio::setXXX methods, no chip specific implementation is needed.
void RADIO::getAudioInfo(AUDIO_INFO *info) {
    // set everything to false and 0.
    memset(info, 0, sizeof(AUDIO_INFO));

    // use current settings
    info->volume = _volume;
    info->mute = _mute;
    info->softmute = _softMute;
    info->bassBoost = _bassBoost;
} // getAudioInfo()




/// Send a 0.0.0.0 to the RDS receiver if there is any attached.
/// This is to point out that there is a new situation and all existing data should be invalid from now on.
void RADIO::clearRDS() { 
    if (_sendRDS)
        _sendRDS(0, 0, 0, 0);
} // clearRDS()


// send valid and good data to the RDS processor via newFunction
// remember the RDS function
void RADIO::attachReceiveRDS(receiveRDSFunction newFunction)
{
    _sendRDS = newFunction;
} // attachReceiveRDS()

// The End.


