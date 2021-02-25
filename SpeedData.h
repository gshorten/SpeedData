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
	
	int getEGO(int readFreq = 50);
	// EGO adjustment, range is 85 - 115. Corresponds to Speeduino max range of +-15%
	
	float getActualAFR(int readFreq = 100);
	// actual air fuel ratio
	
	float getTargetAFR(int readFreq = 100);
	// target air fuel ratio
	
	int getLoops(int readFreq = 200);
	// main loops per second
	
	int getWarmup( int readFreq = 250);
	// warmup enrichment, %
	
	int getMAP( int readFreq = 200);
	// manifold air pressure
	
	int getGammaE (int readFreq = 150);
	// total enrichment adjustments % - cold start, warmup, IAT, etc.
	
	int getAccelEnrich ( int readFreq = 100);
	// acceleration enrichment %
	
	int getData(byte location, byte length);
	// generic function to get data.  Usually wrap this in another function that
	// will set how often to get the data and any data conversion required.

  private:
	Stream *_port;     				
	// Serial port to use on requesting device (ie, ESP32 would be Serial2)
    
	int getSpeeduinoData(byte getData[2]);
	//method that gets the data from the speeduino.
};

#endif
