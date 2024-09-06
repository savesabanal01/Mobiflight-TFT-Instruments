#include "MFCustomDevice.h"
#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFEEPROM.h"
#include <SPI.h>
#ifdef HAS_CONFIG_IN_FLASH
#include "MFCustomDevicesConfig.h"
#else
const char CustomDeviceConfig[] PROGMEM = {};
#endif

extern MFEEPROM MFeeprom;

/* **********************************************************************************
    The custom device pins, type and configuration is stored in the EEPROM
    While loading the config the adresses in the EEPROM are transferred to the constructor
    Within the constructor you have to copy the EEPROM content to a buffer
    and evaluate him. The buffer is used for all 3 types (pins, type configuration),
    so do it step by step.
    The max size of the buffer is defined here. It must be the size of the
    expected max length of these strings.

    E.g. 6 pins are required, each pin could have two characters (two digits),
    each pins are delimited by "|" and the string is NULL terminated.
    -> (6 * 2) + 5 + 1 = 18 bytes is the maximum.
    The custom type is "MyCustomClass", which means 14 characters plus NULL = 15
    The configuration is "myConfig", which means 8 characters plus NULL = 9
    The maximum characters to be expected is 18, so MEMLEN_STRING_BUFFER has to be at least 18
********************************************************************************** */
#define MEMLEN_STRING_BUFFER 40

TFT_eSPI *tft;
// Sprites for Instruments, max. number which can be usedfor an instrument
TFT_eSprite spr[17] = {TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft),
                       TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft),
                       TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft),
                       TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft), TFT_eSprite(tft)};

// reads a string from EEPROM or Flash at given address which is '.' terminated and saves it to the buffer
bool MFCustomDevice::getStringFromMem(uint16_t addrMem, char *buffer, bool configFromFlash)
{
    char     temp    = 0;
    uint8_t  counter = 0;
    uint16_t length  = MFeeprom.get_length();
    do {
        if (configFromFlash) {
            temp = pgm_read_byte_near(CustomDeviceConfig + addrMem++);
            if (addrMem > sizeof(CustomDeviceConfig))
                return false;
        } else {
            temp = MFeeprom.read_byte(addrMem++);
            if (addrMem > length)
                return false;
        }
        buffer[counter++] = temp;              // save character and locate next buffer position
        if (counter >= MEMLEN_STRING_BUFFER) { // nameBuffer will be exceeded
            return false;                      // abort copying to buffer
        }
    } while (temp != '.'); // reads until limiter '.' and locates the next free buffer position
    buffer[counter - 1] = 0x00; // replace '.' by NULL, terminates the string
    return true;
}

MFCustomDevice::MFCustomDevice()
{
    _initialized = false;
}

/* **********************************************************************************
    Within the connector pins, a device name and a config string can be defined
    These informations are stored in the EEPROM or Flash like for the other devices.
    While reading the config from the EEPROM or Flash this function is called.
    It is the first function which will be called for the custom device.
    If it fits into the memory buffer, the constructor for the customer device
    will be called
********************************************************************************** */

void MFCustomDevice::attach(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig, bool configFromFlash)
{
    if (adrPin == 0) return;

    /* **********************************************************************************
        Do something which is required to setup your custom device
    ********************************************************************************** */

    char    *params, *p = NULL;
    char     parameter[MEMLEN_STRING_BUFFER];
    uint8_t  _pin1;
    uint32_t millis_start, millis_end;

    /* **********************************************************************************
        Read the Type from the EEPROM or Flash, copy it into a buffer and evaluate it
        The string get's NOT stored as this would need a lot of RAM, instead a variable
        is used to store the type
    ********************************************************************************** */
    getStringFromMem(adrType, parameter, configFromFlash);
    if (strcmp(parameter, "StandbyAttitudeModule") == 0)
        _customType = STANDBY_ATTITUDE_MONITOR;
    else if (strcmp(parameter, "Airspeed_Indicator") == 0)
        _customType = AIRSPEED_INDICATOR;
    else if (strcmp(parameter, "Attitude_Indicator") == 0)
        _customType = ATTITUDE_INDICATOR;
    else if (strcmp(parameter, "Turn_Coordinator") == 0)
        _customType = TURN_COORDINATOR;
    else if (strcmp(parameter, "Altimeter") == 0)
        _customType = ALTIMETER;
    else if (strcmp(parameter, "Vertical_Speed_Indicator") == 0)
        _customType = VERTICAL_SPEED_INDICATOR;

    if (_customType > 0 && _customType < LAST_INSTRUMENT) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(TFT_eSPI))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("Custom Device does not fit in Memory"));
            return;
        }
        /* **********************************************************************************************
            Read the pins from the EEPROM or Flash, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromMem(adrPin, parameter, configFromFlash);
        /* **********************************************************************************************
            Split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        // params = strtok_r(NULL, "|", &p);
        //_pin2  = atoi(params);
        // params = strtok_r(NULL, "|", &p);
        //_pin3  = atoi(params);

        /* **********************************************************************************
            Read the configuration from the EEPROM or Flash, copy it into a buffer.
        ********************************************************************************** */
        // getStringFromMem(adrConfig, parameter, configFromFlash);
        /* **********************************************************************************
            Split the config up into single parameter. As the number of parameters could be
            different between multiple devices, it is done here.
            This is just an example how to process the init string. Do NOT use
            "," or ";" as delimiter for multiple parameters but e.g. "|"
            For most customer devices it is not required.
            In this case just delete the following
        ********************************************************************************** */
        //uint16_t Parameter1;
        //char    *Parameter2;
        //params     = strtok_r(parameter, "|", &p);
        //Parameter1 = atoi(params);
        //params     = strtok_r(NULL, "|", &p);
        //Parameter2 = params;

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        // In most cases you need only one of the following functions
        // depending on if the constuctor takes the variables or a separate function is required
#if defined(ARDUINO_ARCH_RP2040)
        // Setting SPI clock to processor clock / 2
        // This speeds up SPI transfer for the Pico
        uint32_t freq = clock_get_hz(clk_sys);
        // clk_peri does not have a divider, so in and out frequencies must be the same
        clock_configure(clk_peri,
                        0,
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                        freq,
                        freq);
#endif
        tft = new (allocateMemory(sizeof(TFT_eSPI))) TFT_eSPI();
        tft->init();
        tft->initDMA();
        millis_start = millis();
        tft->fillScreen(TFT_BLACK);
        millis_end = millis();
        tft->setRotation(0);
    } else {
        cmdMessenger.sendCmd(kStatus, F("Custom Device is not supported by this firmware version"));
    }
    if (_customType == STANDBY_ATTITUDE_MONITOR) {
        StandbyAttitudeMonitor::init(tft, spr);
    } else if (_customType == AIRSPEED_INDICATOR) {
        AirspeedIndicator::init(tft, spr);
    } else if (_customType == ATTITUDE_INDICATOR) {
        AttitudeIndicator::init(tft, spr);
    } else if (_customType == TURN_COORDINATOR) {
        TurnCoordinator::init(tft, spr);
    } else if (_customType == ALTIMETER) {
        Altimeter::init(tft, spr);
    } else if (_customType == VERTICAL_SPEED_INDICATOR) {
        VerticalSpeedIndicator::init(tft, spr);
    }

    _initialized = true;
}

/* **********************************************************************************
    The custom devives gets unregistered if a new config gets uploaded.
    Keep it as it is, mostly nothing must be changed.
    It gets called from CustomerDevice::Clear()
********************************************************************************** */
void MFCustomDevice::detach()
{
    _initialized = false;
    if (_customType == STANDBY_ATTITUDE_MONITOR) {
        StandbyAttitudeMonitor::stop();
    } else if (_customType == AIRSPEED_INDICATOR) {
        AirspeedIndicator::stop();
    } else if (_customType == ATTITUDE_INDICATOR) {
        AttitudeIndicator::stop();
    } else if (_customType == TURN_COORDINATOR) {
        TurnCoordinator::stop();
    } else if (_customType == ALTIMETER) {
        Altimeter::stop();
    } else if (_customType == VERTICAL_SPEED_INDICATOR) {
        VerticalSpeedIndicator::stop();
    }
}

/* **********************************************************************************
    Within in loop() the update() function is called regularly
    Within the loop() you can define a time delay where this function gets called
    or as fast as possible. See comments in loop().
    It is only needed if you have to update your custom device without getting
    new values from the connector.
    Otherwise just make a return; in the calling function.
    It gets called from CustomerDevice::update()
********************************************************************************** */
void MFCustomDevice::update()
{
    if (!_initialized) return;

    /* **********************************************************************************
        Do something if required
    ********************************************************************************** */
    if (_customType == STANDBY_ATTITUDE_MONITOR) {
        StandbyAttitudeMonitor::update();
    } else if (_customType == AIRSPEED_INDICATOR) {
        AirspeedIndicator::update();
    } else if (_customType == ATTITUDE_INDICATOR) {
        AttitudeIndicator::update();
    } else if (_customType == TURN_COORDINATOR) {
        TurnCoordinator::update();
    } else if (_customType == ALTIMETER) {
        Altimeter::update();
    } else if (_customType == VERTICAL_SPEED_INDICATOR) {
        VerticalSpeedIndicator::update();
    }
}

/* **********************************************************************************
    If an output for the custom device is defined in the connector,
    this function gets called when a new value is available.
    It gets called from CustomerDevice::OnSet()
********************************************************************************** */
void MFCustomDevice::set(int16_t messageID, char *setPoint)
{
    if (!_initialized) return;

    if (_customType == STANDBY_ATTITUDE_MONITOR) {
        StandbyAttitudeMonitor::set(messageID, setPoint);
    } else if (_customType == AIRSPEED_INDICATOR) {
        AirspeedIndicator::set(messageID, setPoint);
    } else if (_customType == ATTITUDE_INDICATOR) {
        AttitudeIndicator::set(messageID, setPoint);
    } else if (_customType == TURN_COORDINATOR) {
        TurnCoordinator::set(messageID, setPoint);
    } else if (_customType == ALTIMETER) {
        Altimeter::set(messageID, setPoint);
    } else if (_customType == VERTICAL_SPEED_INDICATOR) {
        VerticalSpeedIndicator::set(messageID, setPoint);
    }
}