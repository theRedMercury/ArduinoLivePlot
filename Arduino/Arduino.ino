/*
  Name:		PcMonitorArduino.ino
  Created:	4/4/2020 5:34:32 PM
  Author:  Nicolas Masson
*/

// the setup function runs once when you press reset or power the board


#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library

#include "Axis.h"
#include "LivePlot.h"

LCDWIKI_KBV myLcd(ILI9486, 40, 38, 39, 44, 41); //model,cs,cd,wr,rd,reset

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

String msgS = "";

// CPU
HistoryGraph cpuTotalGraph;
DialGraph cpuTempGraph;
BarGraph cpuPowerGraph;
BarGraph cpuVoltGraph;
BarGraph cpu3VoltGraph;
Axis* cpuTotalAxis = new Axis(1.0, "%", 0.0, 100.0, 0xF800);
Axis* cpuTempAxis = new Axis(0.0, "C", 25.0, 85.0, 0xF800);
Axis* cpuVoltAxis = new Axis(0.0, "Vcore", 0.8, 1.6, 0xFCC6);
Axis* cpu3VoltAxis = new Axis(0.0, "Vcc ", 2.8, 3.6, 0xFCC6);
Axis* cpuPowerAxis = new Axis(0.0, "W cpu", 0.0, 70.0, 0xF800);

// GPU
HistoryGraph gpuTotalGraph;
DialGraph gpuTempGraph;
BarGraph gpuPowerGraph;
Axis* gpuTotalAxis = new Axis(1.0, "%", 0.0, 100.0, 0xF800);
Axis* gpuTempAxis = new Axis(0.0, "C", 25.0, 85.0, 0xF800);
Axis* gpuPowerAxis = new Axis(0.0, "W gpu", 0.0, 120, 0xF800);

// RAM
HistoryGraph ramGraph;
Axis* ramAxis = new Axis(1.0, "%", 0.0, 100.0, 0xFFFF);

// NET
HistoryGraph networkGraph;
Axis* netR = new Axis(0.0, "Kbps (in)", 0xEEEE);
Axis* netS = new Axis(0.0, "Kbps (out)", 0xDDDD);


// FAN
DialGraph fan1Graph;
DialGraph fan2Graph;
DialGraph fan3Graph;
DialGraph fan4Graph;
Axis* fan1Axis = new Axis(0.0, "", 0.0, 1200, 0xD69A);
Axis* fan2Axis = new Axis(0.0, "", 0.0, 1200, 0xBDF7);
Axis* fan3Axis = new Axis(0.0, "", 0.0, 1200, 0xA534);
Axis* fan4Axis = new Axis(0.0, "", 0.0, 1800, 0x8C71); // GPU FAN


String getValueStr(String *data, int index, char separator = ';') {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data->length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data->charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data->substring(strIndex[0], strIndex[1]) : "";
}

void showText(LCDWIKI_KBV* lcd, String txt, uint16_t x, uint16_t y, uint16_t textColor = 0xDEDB, uint16_t textBgColor = 0x0000)
{
  lcd->Set_Text_colour(textColor);
  lcd->Set_Text_Back_colour(textBgColor);
  lcd->Print_String(txt, x, y);
}

void setup()
{
  myLcd.Init_LCD();
  myLcd.Fill_Screen(BLACK);
  myLcd.Set_Rotation(1);

  Serial.begin(115200);

  // CPU ///////////////////////////////////////////////
  cpuTempAxis->setDecimal(1);
  cpuVoltAxis->setDecimal(3);
  cpu3VoltAxis->setDecimal(3);
  cpuTotalGraph.init(&myLcd, 0, 0, 479, 99);
  cpuTotalGraph.setPrimaryAxis(cpuTotalAxis);
  cpuTotalGraph.update(true, false, true, false);

  cpuTempGraph.init(&myLcd, 360, 222, 34);
  cpuTempGraph.setTitle("cpu");
  cpuTempGraph.setPrimaryAxis(cpuTempAxis);
  cpuTempGraph.setBgColor(0x0000);
  cpuTempGraph.update();

  cpuVoltGraph.init(&myLcd, 310, 256, 415, 268);
  cpuVoltGraph.setPrimaryAxis(cpuVoltAxis );
  cpuVoltGraph.update();

  cpu3VoltGraph.init(&myLcd, 310, 273, 415, 285);
  cpu3VoltGraph.setPrimaryAxis(cpu3VoltAxis);
  cpu3VoltGraph.update();

  cpuPowerGraph.init(&myLcd, 310, 290, 426, 302);
  cpuPowerGraph.setPrimaryAxis(cpuPowerAxis);
  cpuPowerGraph.update();


  // GPU ///////////////////////////////////////////////
  gpuTempAxis->setDecimal(1);
  gpuTotalGraph.init(&myLcd, 0, 100, 239, 179);
  gpuTotalGraph.setPrimaryAxis(gpuTotalAxis);
  gpuTotalGraph.update(true, false, false, false);

  gpuTempGraph.init(&myLcd, 440, 222, 34);
  gpuTempGraph.setTitle("gpu");
  gpuTempGraph.setPrimaryAxis(gpuTempAxis);
  gpuTempGraph.setBgColor(0x0000);
  gpuTempGraph.update();

  gpuPowerGraph.init(&myLcd, 310, 307, 426, 319);
  gpuPowerGraph.setPrimaryAxis(gpuPowerAxis);
  gpuPowerGraph.update();

  // RAM ////////////////////////////////////////////////
  ramGraph.init(&myLcd, 240, 100, 479, 179);
  ramGraph.setPrimaryAxis(ramAxis);
  ramGraph.update(true, false, false, false);

  // NETWORK ////////////////////////////////////////////
  networkGraph.init(&myLcd, 0, 250, 220, 319);
  networkGraph.setPrimaryAxis(netR);
  networkGraph.addSecondaryAxis(netS);
  networkGraph.update(true, false, false, true);

  // FAN ///////////////////////////////////////////////
  fan1Graph.init(&myLcd, 36, 222, 34);
  fan1Graph.setPrimaryAxis(fan1Axis);
  fan1Graph.setBgColor(0x0000);
  fan1Graph.setTitle("cpu");

  fan2Graph.init(&myLcd, 112, 222, 34);
  fan2Graph.setPrimaryAxis(fan2Axis);
  fan2Graph.setBgColor(0x0000);
  fan2Graph.setTitle("fan2");

  fan3Graph.init(&myLcd, 188, 222, 34);
  fan3Graph.setPrimaryAxis(fan3Axis);
  fan3Graph.setBgColor(0x0000);
  fan3Graph.setTitle("fan3");

  fan4Graph.init(&myLcd, 264, 222, 34);
  fan4Graph.setPrimaryAxis(fan4Axis);
  fan4Graph.setBgColor(0x0000);
  fan4Graph.setTitle("gpu");
}

void loop()
{
  /*
    cpuL:3.79;1.47;0.75;15.91;5.3;1.52
    cpuLT:4.79;57.5;36.39
    mem:49.02
    gpu:0;33.31;50
    net:0;0
  */

  if (Serial.available()) {
    msgS = Serial.readStringUntil('\r\n');

    if (msgS.startsWith("cpu:")) {
      msgS = getValueStr(&msgS, 1, ':');

      cpuTotalAxis->setValue(getValueStr(&msgS, 0, ';').toFloat());
      cpuTempAxis->setValue(getValueStr(&msgS, 1, ';').toFloat());
      cpuPowerAxis->setValue(getValueStr(&msgS, 2, ';').toFloat());
      cpuVoltAxis->setValue(getValueStr(&msgS, 3, ';').toFloat());
      cpu3VoltAxis->setValue(getValueStr(&msgS, 4, ';').toFloat());
      msgS = "";

      cpuTotalGraph.update(true, false, true, false);
      cpuTempGraph.update();
      cpuPowerGraph.update();
      cpuVoltGraph.update();
      cpu3VoltGraph.update();
      
    }

    if (msgS.startsWith("gpu:")) {
      msgS = getValueStr(&msgS, 1, ':');
      gpuTotalAxis->setValue(getValueStr(&msgS, 0, ';').toFloat());
      gpuTempAxis->setValue(getValueStr(&msgS, 1, ';').toFloat());
      gpuPowerAxis->setValue(getValueStr(&msgS, 2, ';').toFloat());
      msgS = "";
      
      gpuTotalGraph.update(true, false, false, false);
      gpuTempGraph.update();
      gpuPowerGraph.update();
      
    }

    if (msgS.startsWith("mem:")) {
      msgS = getValueStr(&msgS, 1, ':');
      ramAxis->setValue(getValueStr(&msgS, 0, ';').toFloat());
      msgS = "";
      
      ramGraph.update(true, false, false, false);
    }

    if (msgS.startsWith("net:")) {
      msgS = getValueStr(&msgS, 1, ':');
      netR->setValue(getValueStr(&msgS, 0, ';').toFloat());
      netS->setValue(getValueStr(&msgS, 1, ';').toFloat());
      msgS = "";
      
      networkGraph.update(true, false, false, true);
    }

    if (msgS.startsWith("fan:")) {
      msgS = getValueStr(&msgS, 1, ':');
      fan1Axis->setValue(getValueStr(&msgS, 0, ';').toFloat());
      fan2Axis->setValue(getValueStr(&msgS, 1, ';').toFloat());
      fan3Axis->setValue(getValueStr(&msgS, 2, ';').toFloat());
      fan4Axis->setValue(getValueStr(&msgS, 3, ';').toFloat());
      msgS = "";
      
      fan1Graph.update();
      fan2Graph.update();
      fan3Graph.update();
      fan4Graph.update();
    }
    delay(1);
  }
  else {
    delay(5);
  }

}
