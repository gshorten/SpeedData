/*!
    @file SpeedData.cpp
	
	@mainpage Library to get data from a Speeduino secondary interface using ESP32 
	
	@section intro_sec Introduction
	Uses a TTGO ESP32 dev kit module with integrated TFT display, wifi, bluetooth, and two buttons.
	Connects to the Speeduino / Arduino Serial3 and gets data from the Speeduino secondary interface.
	Gets Air Fuel Ratio (AFR), Loops per second, and EGO correction. Also has a generic method
	for getting any other available data.
	
	@section dependencies Dependencies
	SPI.h
	Arduino.h
	
	@section author Author 
	Geoff Shorten
*/

#include "Arduino.h"
#include "SpeedData.h"
#include <SPI.h>

// Class constructor
SpeedData::SpeedData(Stream *port)
{
  _port = port;
}

int SpeedData::getSpeeduinoData(byte getData[2])
{
   /*  Gets data from the speeduino, if it is a two byte word then it joins the bytes into an integer.
      DataStart is the location (offset from 0) of the data in the list, see http://wiki.speeduino.com/en/Secondary_Serial_IO_interface
      noBytes can only be 1 or 2 (ie, a single byte or a two byte word)
      the initiating sequence is sent as a series of 7 bytes
      port is the serial port: Serial2 on the mega, Serial2 on the ESP32
      Call this from other functions that should control how often we read data, average readings, and convert to
      the appropriate formats for display and further processsing.
  */
  byte dataStart = getData[0];
  byte noBytes = getData[1];
  int speedValue = 0;                 	// return value
  
  if (testMode){
	  // we are in test mode, return fake data
	  speedValue = getFakeData(dataStart,.1);
  }
  
  else {
	  // not in test mode, get real data  
	  const byte requestData = 0x72;      	// the letter "r" in hex ,could send the integer or char but being consistent :-)
	  const byte canID = 0x119;           	// speeduino canbus ID; i don't know what this is so I picked one at random.
	  // we are not using canbus but Speeduino expects it 
	  const byte rCommand = 0x30;         	// "r" type command

	  // make two byte hex values to send to speeduino
	  byte startLSB = lowByte(dataStart);   		// high and low bytes for start and length of data sequence
	  byte startMSB = highByte(dataStart);
	  byte lengthLSB = lowByte(noBytes);    	// high ansd low bytes for length of data
	  byte lengthMSB = highByte(noBytes);

	  // make an array of the bytes to send to start transmission of the data
	  byte sendSequence[] = {requestData, canID, rCommand, startLSB, startMSB, lengthLSB, lengthMSB};
	  _port ->write(sendSequence, 7);          // send the sequence to the arduino
	  delay(50);								// try waiting for response
	  byte firstByte = _port->read();
	  if (firstByte == 0x72)                  // 0x72 is confirmation, we apparently have data back, so process it
	  {
		_port->read();            			//next byte in buffer should be the data type confirmation. ignore for now
		if (noBytes == 1)       			// if there is supposed to only be one byte then do another read and save the value
		{
		  speedValue = _port->read();
		}
		else if (noBytes == 2)
		{ 
		  // there are two bytes of data so have to do two reads and join the bytes into an integer
		  byte firstByte = _port->read();
		  byte secondByte = _port->read();
		  speedValue = (secondByte << 8) | firstByte;  //join high and low bytes into integer value
		}
	  }
	  else 
		{
		  speedValue = 0;
		}
  }
	return speedValue;
}

int SpeedData::getData(byte location, byte length){
	// gets data from speeduino one time.
	// get location and length from https://wiki.speeduino.com/en/Secondary_Serial_IO_interface
	byte dataToGet[2] = {location,length};
	long sData = getSpeeduinoData(dataToGet);
	return sData;
}

long SpeedData::getRPM(int freq){
	// get rpm
	byte rpmData[2] = {14,2};
	static long rpm;
	static long lastRead = millis() - 100;

	if (millis() - lastRead > freq) {
		rpm = getSpeeduinoData(rpmData) / 4;
		lastRead = millis();
	}
	return rpm;
}

float SpeedData::getBattVoltage(int freq){
	// get battery voltage
	byte voltData[2] = {9,1};
	static float bVolt;
	static long lastRead = millis() - 100;

	if (millis() - lastRead > freq) {
		bVolt = getSpeeduinoData(voltData) / 10;
		lastRead = millis();
	}
	return bVolt;
}

int SpeedData::getWaterTemp(int freq){
	// get rpm
	byte waterTempData[2] = {7,1};
	static long waterTemp;
	static long lastRead = millis() - 100;
	static boolean firstRead = true;
	

	if (millis() - lastRead > freq) {
		int temp = getSpeeduinoData(waterTempData) - 40;			// range from speeduino is 0 - 255, subtract 40 to get degrees c
		waterTemp = (temp * 1.8) + 32;			// water temp in f.  
		// TODO add a setter to switch between celsius and farenheight
		// check to see if temperature change is reasonable, otherwise it is noise
		//if (firstRead){
			//waterTemp = temp;
			//firstRead = false;
		//}
		//if (abs(temp - waterTemp) < 30){
			//waterTemp = temp;
			//lastRead = millis();
		//}
	}
	return waterTemp;
}

int SpeedData::getTPS(int freq){
	// get rpm
	byte bytesToGet[2] = {24,1};
	static int tps = 0;
	static long lastRead = millis() - 100;

	if (millis() - lastRead > freq) {
		tps = getSpeeduinoData(bytesToGet);
		//tps = map(tps,0,255,0,100);		// convert tps reading into 0 - 100%
		lastRead = millis();
	}
	return tps;
}

int SpeedData::getWarmup(int freq){
	// get warmup enrichment
	byte warmupData[2] = {13,1};
	static long warmup;
	static long lastRead = millis() - 100;

  if (millis() - lastRead > freq) {
    warmup = getSpeeduinoData(warmupData);
	lastRead = millis();
	//Serial.print("Warmup: "); Serial.println(warmup);
  }
  return warmup;
}

int SpeedData::getGammaE(int freq){
	// get total enrichment (Gamma), as percent.  
	byte gammaEData[2] = {17,1};
	static long gammaE;
	static long lastRead = millis() - 100;

  if (millis() - lastRead > freq) {
    gammaE = getSpeeduinoData(gammaEData);
	lastRead = millis();
	//Serial.print("GammaE: "); Serial.println(GammaE);
  }
  return gammaE;
}

int SpeedData::getAccelEnrich(int freq){
	// get Acceleration enrichment, as percent.  
	byte accelEnrichData[2] = {16,1};
	static long accelEnrich;
	static long lastRead = millis() - 100;

  if (millis() - lastRead > freq) {
    accelEnrich = getSpeeduinoData(accelEnrichData);
	lastRead = millis();
	//Serial.print("Acceleration enrichment: "); Serial.println(accelEnrich);
  }
  return accelEnrich;
}

int SpeedData::getMAP(int freq){
	// get manifold air pressure (MAP)
	byte MAPData[2] = {4,2};
	static long map;
	static long lastRead = millis() - 100;

  if (millis() - lastRead > freq) {
    map = getSpeeduinoData(MAPData);
	lastRead = millis();
	//Serial.print("Warmup: "); Serial.println(warmup);
  }
  return map;
}

int SpeedData::getEGO(int freq) {
  /*
   Get the EGO correction.
   100 is no correction, correction is +- from this.  max correction in Speeduino is +- 15%,
   corresponds to range 85 - 115.
   Call this from main loop, or from other function that is in main loop. If not in the main
   loop it will run one time.
   readFreq is how often we get the data (if in the main loop)
   */
  byte egoData[2] = {11,1};      
  static long lastRead = millis() - 100;
  static long EGO;

  if (millis() - lastRead > freq) {
    EGO = getSpeeduinoData(egoData);
    lastRead = millis();
    Serial.print("EGO: "); Serial.println(EGO);
  }
  return EGO;
}

float SpeedData::getActualAFR(int freq) {
  // get actual Air Fuel Ratio (AFR)
  byte actualAFRData[2] = {10, 1};
  static long lastRead = millis();
  float actual;
  static float adjActual;

  if (millis() - lastRead > freq) {
    // get data
    actual = getSpeeduinoData(actualAFRData);
    lastRead = millis();
    adjActual = actual / 10;		// s/b in range of 7.0 - 22.0
  }
  return adjActual;
}

float SpeedData::getTargetAFR (int freq) {
  // get target AFR
  byte targetAFRData[2] = {19, 1};
  static long lastRead = millis();
  float target;
  static float adjTarget;

  if (millis() - lastRead > freq) {
    // get data
    target = getSpeeduinoData(targetAFRData);
    lastRead = millis();
    adjTarget = target / 10;		
  }
  return adjTarget;
}

int SpeedData::getLoops(int freq) {
  // get Speeduino main loops per second . Typically for Speeduino this is ~1100
  byte loopsData[2] = {25,2};          
  static long lastRead = millis();
  static int loopsPS = 1000;

  if (millis() - lastRead > freq) {
    loopsPS = getSpeeduinoData(loopsData);
    lastRead = millis();
  }
  return loopsPS;
}

void SpeedData::testModeOn(){
	testMode = true;
}

void SpeedData::testModeOff(){
	testMode = false;
}	

	
int SpeedData::getFakeData(byte retType, float inc){
	// generates  fake speeduino values along a sine curve  in range appropriate for type of data expected
	int min;
	int max;
	
	// set range for data returned
	switch (retType) {
		case 4:
			// MAP
			min = 0;
			max = 110;
			break;
		case 7:
			// water temperature
			min = 0;
			max = 160;
			break;
		case 9:
			// battery voltage
			min = 0;
			max = 160;
			break;
		case 10:
			//afr
			min = fakeAfrMin;
			max = fakeAfrMax;
			break;
		case 11:
			// EGO correction
			min = 75;
			max = 200;
			break;
		case 13:
			// warmup adder
			min = 100;
			max = 200;
			break;
		case 14:
			// rpm 
			min = 0;
			max = 10000;
			break;
		case 16:
			// Acceleration enrichment
			min = 100;
			max = 200;
			break;
		case 17:
			// GammaE, total adder to vector
			min = 75;
			max = 175;
			break;
		case 22:
			// throttle position (TPS)
			min = 0;
			max =100;
			break;
		case 23:
			// spark advance
			min = 0;
			max = 45;
			break;
		case 25:
			// loops per second
			min = 1000;
			max = 1400;
			break;
		default :
			// AFR;  to get afr divide by 10, so define max min as  your normal values X10
			min = 80;
			max = 220;
			break;
	}
	
	// generate sin curve to simulate fake data
	static float d = 0;
    d += inc; 
	if (d >= 360) { d = inc; }
	float valueRange = (max - min)/2;
    // generate a number in the desired range using the sin value from above
    int value = int(round(sin(d) * valueRange) + (valueRange + min ));
	// constrain if its out of range
	value = constrain(value,min,max);
		
   return value;
}

void SpeedData::setFakeAFR(byte afrMin, byte afrMax){
	// sets afr range to generate
	if (afrMin > afrMax){
		//afrMin must be smaller than afrMax
		//#error afrMin must be smaller than afrMax, resetting to defaults!
		// reset to defaults
		fakeAfrMin = 100;
		fakeAfrMax = 220;
	}
	else {
		fakeAfrMin = afrMin;
		fakeAfrMax = afrMax;
	}
	// add other error checks here for afr range
}
		
