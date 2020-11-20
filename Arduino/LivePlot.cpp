/*
   LivePlot.cpp
  Author:  Nicolas Masson
*/

#include "LivePlot.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float DialGraph::arraysCache[448];  // ((midleAngle::112 * 2) / resolution) * 4

DialGraph::DialGraph() {
  this->axisArray = new Axis * [0];

  // Pre compute rad dial graph
  uint16_t j = 0;
  for (int16_t i = -DialGraph::midleAngle; i < DialGraph::midleAngle; i += resolution) {
    DialGraph::arraysCache[j] = cos((i - 90.0) * DEG2RAD);
    j++;
    DialGraph::arraysCache[j] = sin((i - 90.0) * DEG2RAD);
    j++;
    DialGraph::arraysCache[j] = cos((i + resolution - 90.0) * DEG2RAD);
    j++;
    DialGraph::arraysCache[j] = sin((i + resolution - 90.0) * DEG2RAD);
    j++;
  }
  return;
}

void DialGraph::init(LCDWIKI_KBV* lcd, int16_t x = 0, int16_t y = 0, int16_t radius = 100) {
  this->lcd = lcd;
  this->posX1 = x;
  this->posY1 = y;
  this->radius = radius;
  this->computeGradient(true);
}

int16_t DialGraph::getMax() {
  return DialGraph::midleAngle * 2;
}

void DialGraph::setBgColor(uint16_t color) {
  this->mainColor = color;
  this->computeGradient(true);
}

void DialGraph::setTitle(String title) {
  this->title = title;
}


void DialGraph::update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true) {

  float w = this->radius - (this->radius / 4);
  int16_t v = this->axisArray[0]->getNormalizeValueGui((-DialGraph::midleAngle), (DialGraph::midleAngle));
  bool finalValue = false;

  // Draw colour blocks every inc degrees
  uint16_t j = 0;
  for (int16_t i = -DialGraph::midleAngle; i < DialGraph::midleAngle; i += resolution) {

    // start
    uint16_t x0 = (DialGraph::arraysCache[j] * w) + this->posX1;
    uint16_t y0 = (DialGraph::arraysCache[j + 1] * w) + this->posY1;
    uint16_t x1 = (DialGraph::arraysCache[j] * this->radius) + this->posX1;
    uint16_t y1 = (DialGraph::arraysCache[j + 1] * this->radius) + this->posY1;

    // end
    uint16_t x2 = (DialGraph::arraysCache[j + 2] * w) + this->posX1;
    uint16_t y2 = (DialGraph::arraysCache[j + 3] * w) + this->posY1;
    uint16_t x3 = (DialGraph::arraysCache[j + 2] * this->radius) + this->posX1;
    uint16_t y3 = (DialGraph::arraysCache[j + 3] * this->radius) + this->posY1;
    j += 4;

    if (i < v) { // Fill in coloured segments with 2 triangles
      this->lcd->Set_Draw_color(this->colorGradientBg[i + DialGraph::midleAngle]);
      this->lcd->Fill_Triangle(x0, y0, x1, y1, x2, y2);
      this->lcd->Fill_Triangle(x1, y1, x2, y2, x3, y3);
    }
    else {
      if (!finalValue) {
        this->lcd->Set_Draw_color(this->axisArray[0]->getColor());
        this->lcd->Fill_Triangle(x0, y0, x1, y1, x2, y2);
        this->lcd->Fill_Triangle(x1, y1, x2, y2, x3, y3);
        finalValue = true;
      }
      else // Fill in blank segments
      {
        this->lcd->Set_Draw_color(this->mainColor);
        this->lcd->Fill_Triangle(x0, y0, x1, y1, x2, y2);
        this->lcd->Fill_Triangle(x1, y1, x2, y2, x3, y3);
      }
    }
  }
  if (showValue) {
    this->lcd->Set_Text_colour(this->axisArray[0]->getColor());
    this->lcd->Set_Text_Back_colour(this->mainColor);
    this->lcd->Print_String(this->title, this->posX1 - 8, this->posY1 - 12);
    this->lcd->Print_String(this->axisArray[0]->getFormatString(showUnit), this->posX1 - 8, this->posY1);
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HistoryGraph::HistoryGraph() {
  this->axisArray = new Axis * [2];
  return;
}

void HistoryGraph::init(LCDWIKI_KBV* lcd, int16_t x1 = 0, int16_t y1 = 0, int16_t x2 = 100, int16_t y2 = 100) {
  this->lcd = lcd;
  this->posX1 = x1;
  this->posY1 = y1;
  this->posX2 = x2;
  this->posY2 = y2;

  this->timeValue = this->posX1;

  this->lcd->Set_Draw_color(this->mainColor);
  this->lcd->Fill_Rectangle(this->posX1, this->posY1, this->posX2, this->posY2);
  this->computeGradient();
}

int16_t HistoryGraph::getMax() {
  return this->posY2 - this->posY1;
}

void HistoryGraph::addSecondaryAxis(Axis* axis) {
  this->axisArray[this->axisCount] = axis;
  this->axisCount++;
}

void HistoryGraph::update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true) {

  if (this->axisCount == 0) {
    return;
  }

  this->timeValue++;
  if (this->timeValue > this->posX2) {
    this->timeValue = this->posX1;
  }

  // CleanForward
  this->lcd->Set_Draw_color(this->mainColor);
  for (uint8_t i = 0; i < 5; i++) {
    if (this->timeValue + i <= this->posX2) {
      this->lcd->Draw_Fast_VLine(this->timeValue + i, this->posY1, (this->posY2 - this->posY1) + 1);
    }
  }

  for (uint8_t ac = 0; ac < this->axisCount; ac++) {

    if (this->enableGradientBg && ac == 0) {
      for (int16_t i = this->axisArray[ac]->getNormalizeValueGui(this->posY2, this->posY1); i < this->posY2; i++) {
        this->lcd->Set_Draw_color(this->colorGradientBg[(i - this->posY1)]);
        this->lcd->Draw_Pixel(this->timeValue, i);
      }
    }
    //Draw Point
    this->lcd->Set_Draw_color(this->axisArray[ac]->getColor());
    this->lcd->Draw_Pixel(this->timeValue, this->axisArray[ac]->getNormalizeValueGui(this->posY2, this->posY1));

    //Draw Text
    if (showValue) {
      this->lcd->Set_Text_colour(this->axisArray[ac]->getColor());
      this->lcd->Set_Text_Back_colour(this->mainColor);
      this->lcd->Print_String(this->axisArray[ac]->getFormatString(showUnit), this->posX2 - 24, (this->posY1 + ((this->posY2 - this->posY1) / 2)) + (ac * 12));
    }

    if (ac == 0) {
      if (showMax) {
        this->lcd->Set_Text_colour(this->axisArray[ac]->getMaxColor());
        this->lcd->Set_Text_Back_colour(this->mainColor);
        this->lcd->Print_String(String(this->axisArray[ac]->getMaxValue(), 0), this->posX2 - 24, this->posY1);
      }
      if (showMin) {
        this->lcd->Set_Text_colour(this->axisArray[ac]->getMinColor());
        this->lcd->Set_Text_Back_colour(this->mainColor);
        this->lcd->Print_String(String(this->axisArray[ac]->getMinValue(), 0), this->posX2 - 24 , this->posY2 - 12);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BarGraph::BarGraph(void) {
  this->axisArray = new Axis * [1];
  return;
}

void BarGraph::init(LCDWIKI_KBV* lcd, int16_t x1 = 0, int16_t y1 = 0, int16_t x2 = 100, int16_t y2 = 100) {
  this->lcd = lcd;
  this->posX1 = x1;
  this->posY1 = y1;
  this->posX2 = x2;
  this->posY2 = y2;
}

void BarGraph::setPrimaryAxis(Axis* axis) {
  this->axisArray[0] = axis;
  this->axisCount = 1;
  this->computeGradient(true, axis->getMinColor());
}
void BarGraph::update(bool showValue = true, bool showMin = true, bool showMax = true, bool showUnit = true) {
  this->lcd->Set_Draw_color(this->mainColor);

  uint16_t stepCount = 0;
  uint16_t lastXColor = 0;
  for (uint16_t x = this->posX1; x < this->posX2; x++) {

    if (stepCount < 8) {

      if (x <= this->axisArray[0]->getNormalizeValueGui(this->posX1, this->posX2)) {
        lastXColor = x - this->posX1;
        this->lcd->Set_Draw_color(this->colorGradientBg[lastXColor]);
      }
      else {
        this->lcd->Set_Draw_color(this->mainColor);
      }
      this->lcd->Draw_Fast_VLine(x, this->posY1, (this->posY2 - this->posY1));
    }
    if (stepCount > 14) {
      stepCount = 0;
    }
    stepCount++;
  }

  //Draw Text
  if (showValue) {
    this->lcd->Set_Text_colour(this->colorGradientBg[lastXColor]);
    this->lcd->Set_Text_Back_colour(this->mainColor);
    this->lcd->Print_String(this->axisArray[0]->getFormatString(showUnit), this->posX2, this->posY1 + ((this->posY2 - this->posY1) / 3));
  }
}

int16_t BarGraph::getMax() {
  return this->posX2 - this->posX1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Graph::setPrimaryAxis(Axis* axis) {
  this->axisArray[0] = axis;
  this->axisCount = 1;
  this->computeGradient();
}

void Graph::setEnableGradientBg(bool isEnable = true) {
  this->enableGradientBg = isEnable;
}

void Graph::setBgColor(uint16_t color) {
  this->mainColor = color;
  this->computeGradient();
}

Axis* Graph::getMainAxis() {
  return this->axisArray[0];
}
/*
  fadeRi = R0 + i * (R1 - R0) / N
  fadeGi = G0 + i * (G1 - G0) / N
  fadeBi = B0 + i * (B1 - B0) / N
*/
void Graph::computeGradient(bool revert = false, uint16_t color = NULL) {
  if (this->axisCount == 0) {
    return;
  }

  uint16_t gColor;
  if (color == NULL) {
    gColor = this->mainColor;
  }
  else {
    gColor = color;
  }
  if (colorGradientBg != NULL) {
    delete[] this->colorGradientBg;
    this->colorGradientBg = NULL;
  }

  this->colorGradientBg = new uint16_t[this->getMax()];

  float r0 = ((((this->getMainAxis()->getColor() >> 11) & 0x1F) * 527) + 23) >> 6;
  float g0 = ((((this->getMainAxis()->getColor() >> 5) & 0x3F) * 259) + 33) >> 6;
  float b0 = (((this->getMainAxis()->getColor() & 0x1F) * 527) + 23) >> 6;

  float r1 = ((((gColor >> 11) & 0x1F) * 527) + 23) >> 6;
  float g1 = ((((gColor >> 5) & 0x3F) * 259) + 33) >> 6;
  float b1 = (((gColor & 0x1F) * 527) + 23) >> 6;

  for (int16_t i = 0; i < this->getMax(); i++) {
    uint8_t rg = r0 + i * (r1 - r0) / this->getMax();
    uint8_t gg = g0 + i * (g1 - g0) / this->getMax();
    uint8_t bg = b0 + i * (b1 - b0) / this->getMax();
    if (revert) {
      this->colorGradientBg[(this->getMax() - 1) - i] = (((rg & 0xF8) << 8) | ((gg & 0xFC) << 3) | ((bg & 0xF8) >> 3));  // Color_To_565
    }
    else {
      this->colorGradientBg[i] = (((rg & 0xF8) << 8) | ((gg & 0xFC) << 3) | ((bg & 0xF8) >> 3));  // Color_To_565
    }
  }
}
