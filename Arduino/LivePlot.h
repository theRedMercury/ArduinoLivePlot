/*
   LivePlot.h
  Author:  Nicolas Masson
*/

#ifndef _LIVEPLOT_h
#define _LIVEPLOT_h
#define DEG2RAD 0.0174532925

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <LCDWIKI_KBV.h>
#include "Axis.h"


/*
  Abstract class Graph
*/
class Graph
{
  public:
    virtual void update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true) = 0;

    void setPrimaryAxis(Axis* axis);
    void addSecondaryAxis(Axis* axis) {
      return;
    }
    void setBgColor(uint16_t color);
    void setEnableGradientBg(bool isEnable = true);

  protected:
    virtual int16_t getMax() = 0;

    void computeGradient(bool revert = false, uint16_t color = NULL);
    Axis* getMainAxis();

    int16_t posX1;
    int16_t posY1;

    LCDWIKI_KBV* lcd = NULL;
    Axis** axisArray = NULL;
    uint8_t axisCount = 0;

    uint16_t mainColor = 0x0000;
    uint16_t* colorGradientBg = NULL;
    bool enableGradientBg = true;
};

/*
  Dial Graph
*/
class DialGraph : public Graph {
  public:
    DialGraph(void);
    void init(LCDWIKI_KBV* lcd, int16_t x = 0, int16_t y = 0, int16_t radius = 100);
    void setBgColor(uint16_t color);
    void update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true);

  private:
    int16_t getMax();

    int16_t radius;

    static const int16_t midleAngle = 112;
    static float arraysPreCompute[];
};

/*
  History Graph
*/
class HistoryGraph : public Graph {

  public:
    HistoryGraph(void);
    void init(LCDWIKI_KBV* lcd, int16_t x1 = 0, int16_t y1 = 0, int16_t x2 = 100, int16_t y2 = 100);
    void addSecondaryAxis(Axis* axis);
    void update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true);

  private:
    int16_t getMax();

    int16_t posX2;
    int16_t posY2;
    int16_t timeValue = 0;
};

/*
  Bar Graph
*/
class BarGraph : public Graph {

  public:
    BarGraph(void);
    void init(LCDWIKI_KBV* lcd, int16_t x1 = 0, int16_t y1 = 0, int16_t x2 = 100, int16_t y2 = 100);
    void setPrimaryAxis(Axis* axis);
    void update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true);

  private:
    int16_t getMax();

    int16_t posX2;
    int16_t posY2;
};
#endif
