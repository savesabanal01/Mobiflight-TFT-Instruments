#include "Altimeter.h"

namespace Altimeter
{
#include "./include/altimeter_main.h"
#include "./include/baro_hpa.h"
#include "./include/baro_inhg.h"
#include "./include/needle_100.h"
#include "./include/needle_1000.h"
#include "./include/needle_10000.h"
#include "./include/logo.h"
#include "./include/instrument_bezel.h"

#define PANEL_COLOR 0x7BEE

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite mainSpr        = TFT_eSprite(&tft); // main sprite
    TFT_eSprite altimeterSpr   = TFT_eSprite(&tft); // main sprite
    TFT_eSprite baroSpr        = TFT_eSprite(&tft); // barometer sprite in Hpa
    TFT_eSprite needle10000Spr = TFT_eSprite(&tft); // 10000 needle
    TFT_eSprite needle1000Spr  = TFT_eSprite(&tft); // 1000 needle
    TFT_eSprite needle100Spr   = TFT_eSprite(&tft); // 100 needle

    // Function declarations
    void  drawAll();
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  setAltitude(float value);
    void  setBaro(float value);
    void  setInstrumentBrightnessRatio(float ratio);
    void  setBaroMode(int mode);
    void  setPowerSaveMode(bool enabled);
    void  setScreenRotation(int rotation);

    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
    float   altitude                  = 0;     // altitude value from the simulator
    float   baro                      = 29.92; // barometric pressure value from the simulator
    int     tenThousand               = 0;     // ten thousands value
    int     thousand                  = 0;     // thousands value
    int     hundred                   = 0;     // hundreds value
    float   angleTenThousand          = 0;     // angle for the 10,000 pointer
    float   angleThousand             = 0;     // angle for the 1,000 needle
    float   angleHundred              = 0;     // angle for the 100 needle
    float   angleBaro                 = 0;     // angle of the of the baro indicator base on the baro value
    int     baroMode                  = 0;     // baro mode, 0 = inHG, 1 = hpa, others default to inHG
    int     prevBaroMode              = 0;
    int     instrumentBrightness      = 255; // instrument brightness based on ratio. Value between 0 - 255
    float   instrumentBrightnessRatio = 0;   // previous value of instrument brightness. If no change do not set instrument brightness to avoid flickers
    bool    powerSaveFlag             = false;
    int     screenRotation            = 3;
    int     prevScreenRotation        = 3;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites)
    {
        tft.init();
        tft.setRotation(3);
        tft.fillScreen(PANEL_COLOR);
        tft.setPivot(320, 160);
        tft.setSwapBytes(true);
        tft.pushImage(160, 80, 160, 160, logo);
        delay(3000);
        tft.fillScreen(TFT_BLACK);

        mainSpr.createSprite(320, 320);
        mainSpr.setSwapBytes(false);
        mainSpr.fillSprite(TFT_BLACK);
        mainSpr.setPivot(160, 160);

        altimeterSpr.createSprite(320, 320);
        altimeterSpr.setSwapBytes(false);
        altimeterSpr.fillSprite(TFT_BLACK);
        altimeterSpr.pushImage(0, 0, 320, 320, altimeter_main);
        altimeterSpr.setPivot(160, 160);

        baroSpr.createSprite(320, 320);
        baroSpr.setSwapBytes(true);
        baroSpr.fillSprite(TFT_BLACK);
        baroSpr.setPivot(160, 160);
        baroSpr.pushImage(0, 0, 320, 320, baro_inhg);

        needle10000Spr.createSprite(needle_10000_width, needle_10000_height);
        needle10000Spr.setSwapBytes(true);
        needle10000Spr.fillSprite(TFT_BLACK);
        needle10000Spr.pushImage(0, 0, needle_10000_width, needle_10000_height, needle_10000);
        needle10000Spr.setPivot(needle_10000_width / 2, 141);

        needle1000Spr.createSprite(needle_1000_width, needle_1000_height);
        needle1000Spr.setSwapBytes(true);
        needle1000Spr.fillSprite(TFT_BLACK);
        needle1000Spr.pushImage(0, 0, needle_1000_width, needle_1000_height, needle_1000);
        needle1000Spr.setPivot(needle_1000_width / 2, 90);

        needle100Spr.createSprite(needle_100_width, needle_100_height);
        needle100Spr.setSwapBytes(true);
        needle100Spr.fillSprite(TFT_BLACK);
        needle100Spr.pushImage(0, 0, needle_100_width, needle_100_height, needle_100);
        needle100Spr.setPivot(needle_100_width / 2 - 1, 132);
    }

    void stop()
    {
        mainSpr.deleteSprite();
        altimeterSpr.deleteSprite();
        baroSpr.deleteSprite();
        needle10000Spr.deleteSprite();
        needle1000Spr.deleteSprite();
        needle100Spr.deleteSprite();
    }

    void set(int16_t messageID, char *setPoint)
    {
        /* **********************************************************************************
            Each messageID has it's own value
            check for the messageID and define what to do.
            Important Remark!
            MessageID == -2 will be send from the board when PowerSavingMode is set
                Message will be "0" for leaving and "1" for entering PowerSavingMode
            MessageID == -1 will be send from the connector when Connector stops running
            Put in your code to enter this mode (e.g. clear a display)

        ********************************************************************************** */
        int32_t data = atoi(setPoint);
        // uint16_t output;

        // do something according your messageID
        switch (messageID) {
        case -1:
            // tbd., get's called when Mobiflight shuts down
        case -2:
            // tbd., get's called when PowerSavingMode is entered
        case 0:
            // output = (uint16_t)data;
            // data   = output;
            setAltitude(atof(setPoint));
            break;
        case 1:
            /* code */
            setBaro(atof(setPoint));
            break;
        case 2:
            /* code */
            setInstrumentBrightnessRatio(atof(setPoint));
            break;
        case 100:
            /* code */
            setScreenRotation(atoi(setPoint));
            break;
        default:
            break;
        }
    }

    void update()
    {
        // Do something which is required regulary
        //   if(!powerSaveFlag)
        //   {

        //     if (prevBaroMode != baroMode)
        //     {
        //         if (baroMode == 1)
        //         {
        //             baroSpr.pushImage(0, 0, 320, 320, baro_hpa);
        //         }
        //         else baroSpr.pushImage(0, 0, 320, 320, baro_inhg);

        //         prevBaroMode = baroMode;
        //     }

        //     analogWrite(TFT_BL, instrumentBrightness);

        //     if(prevScreenRotation != screenRotation)
        //     {
        //         tft.setRotation(screenRotation);
        //         prevScreenRotation = screenRotation;
        //     }
        //     drawAll();

        //    }
        //    else digitalWrite(TFT_BL, LOW);

        if (prevBaroMode != baroMode) {
            if (baroMode == 1) {
                baroSpr.pushImage(0, 0, 320, 320, baro_hpa);
            } else
                baroSpr.pushImage(0, 0, 320, 320, baro_inhg);

            prevBaroMode = baroMode;
        }

        analogWrite(TFT_BL, instrumentBrightness);

        if (prevScreenRotation != screenRotation) {
            tft.setRotation(screenRotation);
            prevScreenRotation = screenRotation;
        }
        drawAll();
    }

    void drawAll()
    {

        altimeterSpr.pushImage(0, 0, 320, 320, altimeter_main);

        thousand = (int)(altitude) % 10000;
        hundred  = (int)(altitude) % 1000;

        angleTenThousand = scaleValue(altitude, 0, 100000, 0, 360);
        angleThousand    = scaleValue(thousand, 0, 10000, 0, 360);
        angleHundred     = scaleValue(hundred, 0, 1000, 0, 360);
        if (baroMode == 1) {
            // Baro scale in HPA
            angleBaro = scaleValue(baro, 970, 1050, -109, 109);
        } else { // Baro scale in inHg
            angleBaro = angleBaro = scaleValue(baro, 28.6, 31.1, -120, 112);
        }

        baroSpr.pushRotated(&mainSpr, -angleBaro, TFT_BLACK);
        altimeterSpr.setSwapBytes(true);
        altimeterSpr.pushImage(0, 0, 320, 320, altimeter_main);
        needle10000Spr.pushRotated(&altimeterSpr, angleTenThousand, TFT_BLACK);
        needle1000Spr.pushRotated(&altimeterSpr, angleThousand, TFT_BLACK);
        needle100Spr.pushRotated(&altimeterSpr, angleHundred, TFT_BLACK);

        altimeterSpr.pushToSprite(&mainSpr, 0, 0, TFT_BLACK);

        mainSpr.pushSprite(80, 0);

        mainSpr.fillSprite(TFT_BLACK);
        // baroSpr.fillSprite(TFT_BLACK);
    }

    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void setAltitude(float value)
    {
        altitude = value;
    }

    void setBaro(float value)
    {
        baro = value;
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    }

    void setBaroMode(int mode)
    {
        baroMode = mode;
    }

    void setPowerSaveMode(bool enabled)
    {
        if (enabled) {
            digitalWrite(TFT_BL, LOW);
            powerSaveFlag = true;
        } else {
            analogWrite(TFT_BL, instrumentBrightness);
            powerSaveFlag = false;
        }
    }

    void setScreenRotation(int rotation)
    {
        if (rotation == 1 || rotation == 3)
            screenRotation = rotation;
    }

}