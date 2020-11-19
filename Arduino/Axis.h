/*
   Axis.h
  Author:  Nicolas Masson
*/

#ifndef _AXIS_h
#define _AXIS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


class Axis
{
  public:
    Axis(void);
    Axis(float value, String unit);
    Axis(float value, String unit, uint16_t color);
    Axis(float value, String unit, float minNorm, float maxNorm);
    Axis(float value, String unit, float minNorm, float maxNorm, uint16_t color);


    String getFormatString(bool showUnit = true);
    float getValue();
    float getMinValue();
    float getMaxValue();
    int16_t getNormalizeValueGui(int16_t minValue = 0, int16_t maxValue = 1);
    unsigned char getDecimal();
    uint16_t getColor();
    uint16_t getMinColor();
    uint16_t getMaxColor();

    void setValue(float value);
    void setDecimal(unsigned char value);
    void setNormalise(float minNorm, float maxNorm);
    void setColor(uint16_t color);
    void setColorMinMax(uint16_t colorMin, uint16_t colorMax);
    void setColor(uint16_t color, uint16_t colorMin, uint16_t colorMax);

  private:

    int16_t realMapGui(float x, float inMin, float inMax, int16_t outMin, int16_t outMax);

    String unit = "";
    float value = 0.;
    float realValue = 0.;
    float minValue = NAN;
    float maxValue = NAN;
    unsigned char decimal = 0.;
    uint8_t maxLengthStr = 0;

    bool normalize = false;
    float minNormalize = 0.;
    float maxNormalize = 1.;

    uint16_t color = 0xFFFF;
    uint16_t minColor = 0x07E0;
    uint16_t maxColor = 0xF800;
};
#endif
