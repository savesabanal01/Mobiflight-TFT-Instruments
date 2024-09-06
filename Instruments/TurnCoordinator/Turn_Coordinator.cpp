#include "Turn_Coordinator.h"

namespace TurnCoordinator
{
#include "./include/tc_main_gauge.h"
#include "./include/tc_plane.h"
#include "./include/slip_ball.h"
#include "./include/slip_center_line.h"
#include "./include/logo.h"

#define PANEL_COLOR 0x7BEE

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite TCmainSpr    = TFT_eSprite(&tft); // Main Sprite for Turn Coordinator
    TFT_eSprite TCPlaneSpr   = TFT_eSprite(&tft); // Sprite for Turn Coordinator Airplane
    TFT_eSprite slipBallSpr  = TFT_eSprite(&tft); // Sprite for slip ball
    TFT_eSprite slipCtrLnSpr = TFT_eSprite(&tft); // Sprite for slip center line

    // Functions declrations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  setTurnAngle(float angle);
    void  setInstrumentBrightnessRatio(float ratio);
    void  setSlipAngle(double value);
    void  setScreenRotation(int value);
    void  setPowerSaveMode(bool mode);
    void  drawGauge();

    // Variables
    float turnAngle                     = 0;   // angle of the turn angle from the simulator Initial Value
    float slipAngle                     = 0;   // angle of the slip from the simulator
    float instrumentBrightnessRatio     = 1;   // instrument brightness ratio from sim
    int   instrumentBrightness          = 255; // instrument brightness based on ratio. Value between 0 - 255
    float prevInstrumentBrightnessRatio = 0;   // previous value of instrument brightness. If no change do not set instrument brightness to avoid flickers
    float ballXPos                      = 0;   // X position of the ball
    float ballYPos                      = 0;   // YPosition of the ball
    bool  powerSaveFlag                 = false;
    int   screenRotation                = 3;
    int   prevScreenRotation            = 3;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites)
    {

        tft.setRotation(3);
        tft.fillScreen(PANEL_COLOR);
        tft.setPivot(320, 160);
        tft.setSwapBytes(true);
        tft.pushImage(160, 80, 160, 160, logo);
        delay(3000);
        tft.fillScreen(TFT_BLACK);

        TCmainSpr.createSprite(320, 320);
        TCmainSpr.setSwapBytes(true);
        TCmainSpr.fillSprite(TFT_BLACK);
        TCmainSpr.pushImage(0, 0, 320, 320, tc_main_gauge);

        slipBallSpr.createSprite(slip_ball_width, slip_ball_height);
        slipBallSpr.setSwapBytes(false);
        slipBallSpr.fillSprite(TFT_BLACK);
        slipBallSpr.pushImage(0, 0, slip_ball_width, slip_ball_height, slip_ball);

        slipCtrLnSpr.createSprite(slip_center_line_width, slip_center_line_height);
        slipCtrLnSpr.setSwapBytes(false);
        slipCtrLnSpr.fillSprite(TFT_BLACK);
        slipCtrLnSpr.pushImage(0, 0, slip_center_line_width, slip_center_line_height, slip_center_line);

        TCPlaneSpr.createSprite(tc_plane_width, tc_plane_height);
        TCPlaneSpr.setSwapBytes(true);
        TCPlaneSpr.fillSprite(TFT_BLACK);
        TCPlaneSpr.pushImage(0, 0, tc_plane_width, tc_plane_height, tc_plane);
        TCPlaneSpr.setPivot(tc_plane_width / 2, 36);
    }

    void stop()
    {
        TCmainSpr.deleteSprite();
        TCPlaneSpr.deleteSprite();
        slipBallSpr.deleteSprite();
        slipCtrLnSpr.deleteSprite();
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
        int32_t  data = atoi(setPoint);
        uint16_t output;

        // do something according your messageID
        switch (messageID) {
        case -1:
            // // tbd., get's called when Mobiflight shuts down
            setPowerSaveMode(true);
        case -2:
            // // tbd., get's called when PowerSavingMode is entered
            if (data == 1)
                setPowerSaveMode(true);
            else if (data == 0)
                setPowerSaveMode(false);
        case 0:
            // output = (uint16_t)data;
            // data   = output;
            setTurnAngle(atof(setPoint));
            break;
        case 1:
            /* code */
            setSlipAngle(atof(setPoint));
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

        //     analogWrite(TFT_BL, instrumentBrightness);

        //     if(prevScreenRotation != screenRotation)
        //     {
        //         tft.setRotation(screenRotation);
        //         prevScreenRotation = screenRotation;
        //     }
        //     drawGauge();

        //    }
        //    else digitalWrite(TFT_BL, LOW);

        analogWrite(TFT_BL, instrumentBrightness);

        if (prevScreenRotation != screenRotation) {
            tft.setRotation(screenRotation);
            prevScreenRotation = screenRotation;
        }
        drawGauge();
    }

    void drawGauge()
    {

        TCmainSpr.setPivot(160, 160);
        TCmainSpr.pushImage(0, 0, 320, 320, tc_main_gauge);

        ballXPos = scaleValue(slipAngle, 8, -8, 81, 209);
        ballYPos = cos((scaleValue(slipAngle / 2, 8, -8, -116, -244) - (-180)) * DEG_TO_RAD) * 36 + 153; // Approximation based on trial and error
        slipBallSpr.pushToSprite(&TCmainSpr, ballXPos, ballYPos, TFT_BLACK);

        slipCtrLnSpr.pushToSprite(&TCmainSpr, 144, 190, TFT_BLACK);

        TCPlaneSpr.pushRotated(&TCmainSpr, turnAngle, TFT_BLACK);

        TCmainSpr.pushSprite(80, 0);
    }

    void setTurnAngle(float angle)
    {
        turnAngle = angle;
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    }

    void setSlipAngle(double value)
    {
        slipAngle = value;
    }

    void setScreenRotation(int rotation)
    {
        if (rotation == 1 || rotation == 3)
            screenRotation = rotation;
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

    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

}