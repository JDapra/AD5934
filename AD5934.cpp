#include<AD5934.h>
#include<Arduino.h>

AD5934::AD5934(int ClockPin, long ExtClock) {
	Wire.begin();
	pinMode(ClockPin, OUTPUT);
	_extClock = ExtClock;
	//setExtClock(true);
	//Timer1.initialize(1e6/_extClock);
	//Timer1.pwm(ClockPin, 512);
}
/*Status functions
The following set of functions define the status of the AD5934 and are typically set to the Control Registers 0x80 and 0x81. The following functions are available:
reSet()				writes bit D4 and should place device in standby mode. (needs confirmation);
standBy()			writes 0xB0 to 0x80 (control register 1) without changing the voltage range, gain or clock settings.
powerDown() 		writes 0xA0 to control register 1. The device should be placed in this mode when it is not in use to reduce energy requirement.
initializeSweep() 	this function enables the frequency output with the programmed start frequency. The output will remain on indefinitely until a sweep is started (typically after a user defined settling time.
startSweep()		a frequency sweep with the programmed parameters for settlings cycles, start frequency, frequency increment and number of increments is started. Values for the complex impedance will be stored in the according registers. The status register must be polled to know if valid data are available in the storage registers and if the sweep is complete. The jump to the next higher frequency has to be triggered with the function incrementFrequency()
repeatFrequency()	measures the same frequency again to allow averaging
incrementFrequency()	sets the output to the next higher frequency and records result data in real and imaginary storage registers
*/

void AD5934::reSet() {
	writeData(CtrlReg2, 0x10);
}

void AD5934::standBy() {
  // Standby '10110000' Mask D8-10 to avoid tampering with range, gain or clock
  writeData(CtrlReg1, (readData(CtrlReg1) & 0x07) | 0xB0);
  #if debug
  Serial.println("Standby");
  #endif
}

void AD5934::powerDown() {
  writeData(CtrlReg1, 0xA0);
  Serial.println("Power down");
}

void AD5934::initializeSweep() {
  writeData(CtrlReg1, (readData(CtrlReg1) & 0x07) | 0x10);
  Serial.println("Initialised with start frequency");
}

void AD5934::startSweep() {
  writeData(CtrlReg1, (readData(CtrlReg1) & 0x07 | 0x20));
  Serial.println("Start frequency sweep");
}

void AD5934::repeatFrequency() {
  writeData(CtrlReg1, (readData(CtrlReg1) & 0x07 )| 0x40);
  #if debug
  Serial.println("Repeat frequency");
  #endif
}

void AD5934::incrementFrequency(){
  writeData(CtrlReg1, (readData(CtrlReg1) & 0x07) | 0x30);
  #if debug
  Serial.println("Increment frequency");
  #endif
}


int AD5934::readData(int address) {
  int data;
  Wire.beginTransmission(SlaveAddress);
  Wire.write(AddrPointer);
  Wire.write(address);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(SlaveAddress, 1);
  if (Wire.available() >= 1) {
    data = Wire.read();
  }
  else {
    data = -1;
  }
  delay(1);
  return data;
}


void AD5934::writeData(int address, int data) {
  Wire.beginTransmission(SlaveAddress);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
  delay(1);
} 


void AD5934::setRange(int range) {
  //reset D10 and D9
  writeData(CtrlReg1, readData(CtrlReg1) & 0xF9);
  //set the voltage range:
  switch (range) {
    // range 1: 2.0 Vp-p
    case 1:
      writeData(CtrlReg1, readData(CtrlReg1) | 0x00);
      break;

    // range 2: 1.0 Vp-p
    case 2:
      writeData(CtrlReg1, readData(CtrlReg1) | 0x06);
      break;

    // range 3: 200 mVp-p
    case 3:
      writeData(CtrlReg1, readData(CtrlReg1) | 0x02);
      break;

    // range 4: 400 mVp-p
    case 4:
      writeData(CtrlReg1, readData(CtrlReg1) | 0x04);
      break;

    default:
      break;
  }
}

void AD5934::setPGA(int PGA) {
  //clear D8
  writeData(CtrlReg1, readData(CtrlReg1) & 0xFE);

  switch (PGA) {
    //set programmable gain to 1
    case 1:
      writeData(CtrlReg1, readData(CtrlReg1) | 0x01);
      break;

    //set programmable gain to 5
    case 5:
      writeData(CtrlReg1, readData(CtrlReg1) | 0x00);
      break;

    default:
      break;
  }
}

void AD5934::setExtClock(bool clk) {
  byte cl;
  if (clk) {
    cl = 0x04; //Use external clock
    _opClock = _extClock;
  }
  else {
    cl = 0x00; //Use internal clock
    _opClock = 16776000;
  }
  //write to control register
  return writeData(CtrlReg2, cl);
}

int AD5934::setStartFrequency(float freq) {
	_StartFreq = freq;
	writeData(StartFreq1, getFrequency(freq, 1));
	writeData(StartFreq2, getFrequency(freq, 2));
	writeData(StartFreq3, getFrequency(freq, 3));
}

byte AD5934::getFrequency(float freq, int n) {
  long val = long((freq / (_opClock / 16)) * pow(2, 27));
  byte code;
  if (val > 0xFFFFFF) {
    Serial.println("Frequency too high!");
	Serial.println(freq);
  }
  else {
    switch (n) {
      case 1:
        code = (val & 0xFF0000) >> 0x10;
        break;

      case 2:
        code = (val & 0x00FF00) >> 0x08;
        break;

      case 3:
        code = (val & 0x0000FF);
        break;

      default:
        code = 0;
    }
  }
  return code;
}

int AD5934::setFrequencyIncrement(float freq) {
	_FreqIncrement = freq;
	writeData(FreqIncrement1, getFrequency(freq, 1));
	writeData(FreqIncrement2, getFrequency(freq, 2));
	writeData(FreqIncrement3, getFrequency(freq, 3));
}

int AD5934::setNumberIncrements(int nInc) {
	_NumberIncrements = nInc;
  writeData(NumberIncrements1, (nInc & 0x001F00) >> 0x08);
  writeData(NumberIncrements2, (nInc & 0x0000FF));
}

int AD5934::measureZ(int nRepeats){
	int n = 0;
	int i = 1;
	float f;
	float magnitude;
	float phase;
	//Print header line
	Serial.println("Frequency\tMagnitude\tPhase\tResistance\tReactance");
	//Check if frequency sweep is completed
	while (i < nRepeats || (readData(StatusReg) & 0x04) !=0x04){
		//Pause between measurements
		delay(100);
		//If valid data are available, print them to serial out
		if((readData(StatusReg) & 0x02) == 2){
			//Read real register
			byte Re1 = readData(ReData1);
			byte Re2	= readData(ReData2);
			float real = (Re1 << 8) | Re2;
						
			//Read imaginary register
			byte Im1 = readData(ImData1);
			byte Im2 = readData(ImData2);
			float imag = (Im1 << 8) | Im2;
			
			//Calculate current frequency, impedance magnitude and phase
			f = _StartFreq + n*_FreqIncrement;
			magnitude = sqrt(pow(real,2) + pow(imag,2));
			phase = atan(imag/real)*180/3.141592;
			
			//Print results
			Serial.print(f);
			Serial.print("\t");
			Serial.print(magnitude,3);
			Serial.print("\t");
			Serial.print(phase,1);
			Serial.print("\t");
			Serial.print((int)real);
			//Serial.print(Re1,HEX);
			//Serial.print(Re2,HEX);
			Serial.print("\t");
			Serial.println((int)imag);
			//Serial.print(Im1,HEX);
			//Serial.println(Im2,HEX);
			
			if (i < nRepeats){
			repeatFrequency();
			i++;
			}
			else{
				//Next frequency
				if((readData(StatusReg) & 0x07) < 4 ){
					incrementFrequency();
					n++;
					if ( n == _NumberIncrements){
						i = 0;
					}
					else {
						i = 1;
					}
				}
			}
		}
	}
	Serial.println("Done");
}

int AD5934::measureZnew(int nRepeats){
	int n = 0;	//Counter for frequency calculations
	int i = 1;	//Counter for repeats
	const long t0 = millis(); 	//starting time reference
	long t1 = 0;				//time of respective measurement
	int interResult [2][nRepeats];	//Array with intermediate results for averaging
	float f;
	bool flag;
	//Print header line
	Serial.println("Time\tFrequency\tMagnitude\tPhase\tResistance\tReactance");
	//Check if frequency sweep is completed
	while (i < nRepeats || (readData(StatusReg) & 0x04) !=0x04 || flag !=true){
		//Pause between measurements
		delay(100);
		//If valid data are available, print them to serial out
		if((readData(StatusReg) & 0x02) == 2){
			//Read real register
			byte Re1 = readData(ReData1);
			byte Re2 = readData(ReData2);
			interResult [0][n] = (Re1 << 8) | Re2; 	//Write real value to first column in array
						
			//Read imaginary register
			byte Im1 = readData(ImData1);
			byte Im2 = readData(ImData2);
			interResult [1][n] = (Im1 << 8) | Im2;	//Write imaginary value to second column in array
			
			if (i < nRepeats){
			repeatFrequency();
			i++;
			flag = false;
			}
			else{
				flag = true;
				f = _StartFreq + n*_FreqIncrement;
				//Next frequency
				if((readData(StatusReg) & 0x07) < 4 ){
					incrementFrequency();
					n++;
					if ( n == _NumberIncrements){
						i = 0;
					}
					else {
						i = 1;
					}
				}
			}
			if (flag){
			//Calculate average values for real and imaginary values
			long sum[] = {0,0};	//Array of sums of intermediate results. Column 1 = real, column 2 = imaginary
			for (int m = 0; m < nRepeats; m++){
				sum[0]+= interResult[0][m];
				sum[1]+= interResult[1][m];
			}
			float real = sum[0]/nRepeats;
			float imag = sum[1]/nRepeats;
			
			//Calculate current frequency, impedance magnitude and phase
			
			float magnitude = sqrt(square(real) + square(imag));
			float phase = atan(imag/real)*180/3.141592;
			
			//Get time elapsed since entering this routine
			t1 = millis() - t0;
			
			//Print results
			Serial.print(t1);
			Serial.print("\t");
			Serial.print(f);
			Serial.print("\t");
			Serial.print(magnitude,3);
			Serial.print("\t");
			Serial.print(phase,1);
			Serial.print("\t");
			Serial.print((int)real);
			//Serial.print(Re1,HEX);
			//Serial.print(Re2,HEX);
			Serial.print("\t");
			Serial.println((int)imag);
			//Serial.print(Im1,HEX);
			//Serial.println(Im2,HEX);
			}
		}
	}
	Serial.println("Done");
}


int AD5934::setSettlingCycles(int nCyc){
  int lowerHex = nCyc % 256;
  int upperHex = ((nCyc - lowerHex)>> 8) % 2;
  writeData(NumberSettlingCycles1, upperHex);
  writeData(NumberSettlingCycles2, lowerHex);
}

bool AD5934::readDFTStatus(){
  while ((readData(StatusReg) & 0x02) != 0x02){
    delay(100);
  }
}

bool AD5934::readSweepStatus(){
  while ((readData(StatusReg) & 0x04) != 0x04){
    incrementFrequency();
  }
}
			