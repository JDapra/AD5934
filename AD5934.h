#ifndef AD5934_header
#define AD5934_header

#include<Wire.h>
#include<Arduino.h>

const int SlaveAddress = 0x0D;
const int AddrPointer = 0xB0;

const int CtrlReg1 = 0x80;
const int CtrlReg2 = 0x81;

const int StartFreq1 = 0x82;
const int StartFreq2 = 0x83;
const int StartFreq3 = 0x84;

const int FreqIncrement1 = 0x85;
const int FreqIncrement2 = 0x86;
const int FreqIncrement3 = 0x87;

const int NumberIncrements1 = 0x88;
const int NumberIncrements2 = 0x89;

const int NumberSettlingCycles1 = 0x8A;
const int NumberSettlingCycles2 = 0x8B;

const int ReData1 = 0x94;
const int ReData2 = 0x95;

const int ImData1 = 0x96;
const int ImData2 = 0x97;

const int StatusReg = 0x8F;
//#define debug true



class AD5934{
	public:
		AD5934();
		int ClockPin;
		//int ExtClock;
		int readData(int);
		void writeData(int, int);
		
		void reSet();
		void standBy();
		void powerDown();
		void initializeSweep();
		void startSweep();
		void repeatFrequency();
		void incrementFrequency();
		
		void setRange(int);
		void setPGA(int);
		void setStartFrequency(float);
		void setFrequencyIncrement(float);
		void setNumberIncrements(int);
		void setSettlingCycles(int);
		void setExtClock(int, int);

		int measureZ(int);
		//int measureZnew(int);
		bool readDFTStatus();
		bool readSweepStatus();
	private:
		long _Clock;
		float _StartFreq;
		float _FreqIncrement;
		int _NumberIncrements;
		byte _getFrequency(float, int);
};
		
#endif