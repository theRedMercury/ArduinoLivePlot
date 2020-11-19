/*
   Axis.cpp
  Author:  Nicolas Masson
*/

#include "Axis.h"


Axis::Axis(void) {
  return;
}
Axis::Axis(float value, String unit) {
  this->setValue(value);
  this->unit = unit;
}
Axis::Axis(float value, String unit, uint16_t color) {
  this->color = color;
  this->setValue(value);
  this->unit = unit;
}
Axis::Axis(float value, String unit, float minNorm, float maxNorm) {
  this->minNormalize = minNorm;
  this->maxNormalize = maxNorm;
  this->normalize = true;
  this->setValue(value);
  this->unit = unit;
}
Axis::Axis(float value, String unit, float minNorm, float maxNorm, uint16_t color) {
  this->minNormalize = minNorm;
  this->maxNormalize = maxNorm;
  this->normalize = true;
  this->color = color;
  this->setValue(value);
  this->unit = unit;
}


String Axis::getFormatString(bool showUnit = true) {
  String formatStr;
  if (showUnit) {
    formatStr = String(this->realValue, this->decimal) + " " + this->unit;
  }
  else {
    formatStr = String(this->realValue, this->decimal);
  }

  if (formatStr.length() > maxLengthStr) {
    maxLengthStr = formatStr.length();
  }
  if (maxLengthStr > formatStr.length()) {
    for (uint8_t i = formatStr.length(); i < maxLengthStr; i++) {
      formatStr += " ";
    }
  }
  return formatStr;
}
float Axis::getValue() {
  return this->value;
}

float Axis::getMinValue() {
  return this->minValue;
}

float Axis::getMaxValue() {
  return this->maxValue;
}

unsigned char  Axis::getDecimal() {
  return this->decimal;
}

uint16_t  Axis::getColor() {
  return this->color;
}
uint16_t  Axis::getMinColor() {
  return this->minColor;
}
uint16_t  Axis::getMaxColor() {
  return this->maxColor;
}

int16_t Axis::getNormalizeValueGui(int16_t minValue = 0, int16_t maxValue = 1) {
  if (this->normalize) {
    return this->realMapGui(this->value, this->minNormalize, this->maxNormalize, minValue, maxValue);
  }
  return this->realMapGui(this->value, this->minValue, this->maxValue, minValue, maxValue);
}

int16_t Axis::realMapGui(float x, float inMin, float inMax, int16_t outMin, int16_t outMax)
{
  float f1 = (x - inMin) * (outMax - outMin);
  int16_t f2 = f1 / (float)(inMax - inMin);
  return f2 + outMin;
}

void Axis::setValue(float value) {
  this->realValue = value;
  if (this->normalize) {
    if (value < this->minNormalize) {
      value = this->minNormalize;
    }
    if (value > this->maxNormalize) {
      value = this->maxNormalize;
    }
  }
  this->value = value;

  if (isnan(this->minValue))
  {
    this->minValue = this->realValue;
  }
  if (isnan(this->maxValue))
  {
    this->maxValue = this->realValue;
  }
  if (this->realValue < this->minValue) {
    this->minValue = this->realValue;
  }
  if (this->realValue > this->maxValue) {
    this->maxValue = this->realValue;
  }
}

void Axis::setNormalise(float minNorm, float maxNorm) {
  this->minNormalize = minNorm;
  this->maxNormalize = maxNorm;
  this->normalize = true;
}

void Axis::setColor(uint16_t color) {
  this->color = color;
}

void Axis::setColorMinMax(uint16_t colorMin, uint16_t colorMax) {
  this->minColor = colorMin;
  this->maxColor = colorMax;
}

void Axis::setColor(uint16_t color, uint16_t colorMin, uint16_t colorMax) {
  this->color = color;
  this->minColor = colorMin;
  this->maxColor = colorMax;
}

void Axis::setDecimal(unsigned char value) {
  this->decimal = value;
}
