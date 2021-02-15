/* Example of getting data from Speeduino secondary interface.
   On an Arduino Mega based Speeduino connect to the Serial3 (pins D14, D15)
   Use any serial on your device board, on ESP32 I used Serial2
*/

#include <SpeedData.h>

// create SpeedData object to get data
SpeedData SData;

// pins to use for serial2 on ESP32
#define sTX 21
#define sRX 22

void setup() {
  Serial.begin(115200);
  // set up second serial port to connect to the Speeduino Mega 2560, pins D14 (TX3) and D15 (RX3)
  Serial2.begin(115200, SERIAL_8N1, sRX, sTX);
  // set the serial port for SpeedData class to  use
  SData.setSerial(&Serial2);
}

void loop() {
  // loop and get AFR data.  By default it gets new data every 50ms, in this example we get every 500ms
  // can only get data as fast as main loop
  float actualAFR = SData.getActualAFR(500);
  float targetAFR = SData.getTargetAFR(500);
  Serial.print("Current AFR is: ");Serial.println(actualAFR);
  Serial.print("Target AFR is: ");Serial.println(targetAFR);
  // delay so we can read the serial monitor. Normally you would not have a delay in here.
  delay(500);
}
