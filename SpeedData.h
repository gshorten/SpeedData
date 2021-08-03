/*!
	@file SpeedData.h
	
   Library to get data out from speeduino ECU  from another microcontroller (ESP32, etc).
   Data is sent from the Speeduino from Secondary serial interface.
   Gets AFR,EGO, and Loops per second (LPS), Also a generic function to get any data.  Documentation on secondary interface here:
   https://wiki.speeduino.com/en/Secondary_Serial_IO_interface 
*/

#ifndef SpeedData_h
#define SpeedData_h

#include "Arduino.h"
#include <SPI.h>

/*!
@brief Class to get data from the speeduino
*/

class SpeedData
{
  public:
    SpeedData(Stream *port);   // constructor
	// specify port with &, ie &Serial2 for Serial 2 port
	// This would use Serial2 on the device (ie, ESP32) to request and receive data from
	// the Speeduino Ardruino Mega.

	// Methods to get data.  These should be in the main loop or a function called from the main loop.
	// readFreq parameter is in millis, it sets how often the return variable is updated.
	
	long getRPM(int freq = 250);
	// rpm
	
	float getBattVoltage(int freq = 500);
	// battery voltage
	
	int getWaterTemp(int freq = 1000);
	// water temperature
	
	int getTPS(int freq = 250);
	// get throttle position (TPS)
	
	int getEGO(int freq = 100);
	// EGO adjustment, range is 85 - 115. Corresponds to Speeduino max range of +-15%
	
	float getActualAFR(int freq = 100);
	// actual air fuel ratio
	
	float getTargetAFR(int freq = 100);
	// target air fuel ratio
	
	int getLoops(int freq = 200);
	// main loops per second
	
	int getWarmup(int freq = 250);
	// warmup enrichment, %
	
	int getMAP(int freq = 200);
	// manifold air pressure
	
	int getGammaE (int freq = 200);
	// total enrichment adjustments % - cold start, warmup, IAT, etc.
	
	int getAccelEnrich (int freq = 250);
	// acceleration enrichment %
	
	int getData(byte location, byte length);
	// generic function to get data.  Usually wrap this in another function that
	// will set how often to get the data and any data conversion required.
	
	void testModeOn();
	// turns test mode on, instead of real speeduino data simulated data will be used
	
	void testModeOff();
	// turns test mode off, go back to getting data from speeduino
	
	void setFakeAFR(byte min = 120, byte max = 180);
	// sets range for fake (ie, simulated) air fuel ratio data
	// multiply desired min and max afr simulated data by 10, round, pass min and max as pararmeters in setter.
	
	
  private:
	Stream *_port;     				
	// Serial port to use on requesting device (ie, ESP32 would be Serial2)
    
	boolean testMode = false;
	// flag; either test mode, with fake data, or not - with actual speeduino data
	
	int getSpeeduinoData(byte getData[2]);
	//method that gets the data from the speeduino.
	
		
	int getFakeData(byte retType, float inc);
	// generates sequential sine curve data - ie, simulated, fake data
	
	byte fakeAfrMin = 110;
	byte fakeAfrMax = 185;
	// global variable for simulated afr data range.  
		
};

#endif
