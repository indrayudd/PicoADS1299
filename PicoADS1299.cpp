//
//  ADS1299DIAISY.cpp   ARDUINO LIBRARY FOR COMMUNICATING WITH TWO
//  DAISY-CHAINED ADS1299 BOARDS
//  
//  Created by Conor Russomanno, Luke Travis, and Joel Murphy. Summer, 2013
//
//  Extended by Chip Audette through April 2014
//
// Modified by Indrayudd Roy Chowdhury through March 2023


//#include "pins_arduino.h"
#include "PicoADS1299.h"

void PicoADS1299::initialize(int _DRDY, int _RST, int _CS, int _FREQ, boolean _isDaisy){
	isDaisy = _isDaisy;
	DRDY = _DRDY;
	CS = _CS;
	int FREQ = _FREQ;
	int RST = _RST;
	
	delay(50);				// recommended power up sequence requiers Tpor (~32mS)	
	pinMode(RST,OUTPUT);
	pinMode(RST,LOW);
	delayMicroseconds(4);	// toggle reset pin
	pinMode(RST,HIGH);
	delayMicroseconds(20);	// recommended to wait 18 Tclk before using device (~8uS);
	

    // **** ----- SPI Setup ----- **** //

    if (! PicoSPI0.configure (2, 3, 4, 5, FREQ*1000000ul, 1, false))
    {
        while (true) {}
    }
    Serial.println("SPI Established.");
    
    
    
    
    
    // **** ----- End of SPI Setup ----- **** //
    
    // initalize the  data ready chip select and reset pins:
    pinMode(DRDY, INPUT);
    // pinMode(CS, OUTPUT);
	
	// digitalWrite(CS,HIGH);
	digitalWrite(RST,HIGH);
}

//System Commands
void PicoADS1299::WAKEUP() {
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_WAKEUP);
    PicoSPI0.endTransaction(); 
    delayMicroseconds(3);  		//must wait 4 tCLK cycles before sending another command (Datasheet, pg. 35)
}

void PicoADS1299::STANDBY() {		// only allowed to send WAKEUP after sending STANDBY
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_STANDBY);
    PicoSPI0.endTransaction();
}

void PicoADS1299::RESET() {			// reset all the registers to default settings
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_RESET);
    delayMicroseconds(12);   	//must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    PicoSPI0.endTransaction();
}

void PicoADS1299::START() {			//start data conversion 
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_START);
    PicoSPI0.endTransaction();
}

void PicoADS1299::STOP() {			//stop data conversion
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_STOP);
    PicoSPI0.endTransaction();
}

void PicoADS1299::RDATAC() {
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_RDATAC);
    PicoSPI0.endTransaction();
	delayMicroseconds(3);   
}
void PicoADS1299::SDATAC() {
    PicoSPI0.beginTransaction();
    
    PicoSPI0.transfer(_SDATAC);
    PicoSPI0.endTransaction();
	delayMicroseconds(3);   //must wait 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}


// Register Read/Write Commands
byte PicoADS1299::getDeviceID() {			// simple hello world com check
	byte data = RREG(0x00);
	if(verbose){						// verbose otuput
		Serial.print(F("Device ID "));
		printHex(data);	
	}
	return data;
}

byte PicoADS1299::RREG(byte _address) {		//  reads ONE register at _address
    byte opcode1 = _address + 0x20; 	//  RREG expects 001rrrrr where rrrrr = _address
    PicoSPI0.beginTransaction(); 				//  open SPI
    
    PicoSPI0.transfer(opcode1); 					//  opcode1
    PicoSPI0.transfer(0x00); 					//  opcode2
    regData[_address] = PicoSPI0.transfer(0x00);//  update mirror location with returned byte
    PicoSPI0.endTransaction(); 			//  close SPI	
	if (verbose){						//  verbose output
		printRegisterName(_address);
		printHex(_address);
		Serial.print(", ");
		printHex(regData[_address]);
		Serial.print(", ");
		for(byte j = 0; j<8; j++){
			Serial.print(bitRead(regData[_address], 7-j));
			if(j!=7) Serial.print(", ");
		}
		
		Serial.println();
	}
	return regData[_address];			// return requested register value
}

// Read more than one register starting at _address
void PicoADS1299::RREGS(byte _address, byte _numRegistersMinusOne) {
//	for(byte i = 0; i < 0x17; i++){
//		regData[i] = 0;					//  reset the regData array
//	}
    byte opcode1 = _address + 0x20; 	//  RREG expects 001rrrrr where rrrrr = _address
    PicoSPI0.beginTransaction(); 				//  open SPI
    
    PicoSPI0.transfer(opcode1); 					//  opcode1
    PicoSPI0.transfer(_numRegistersMinusOne);	//  opcode2
    for(int i = 0; i <= _numRegistersMinusOne; i++){
        regData[_address + i] = PicoSPI0.transfer(0x00); 	//  add register byte to mirror array
		}
    PicoSPI0.endTransaction(); 			//  close SPI
	if(verbose){						//  verbose output
		for(int i = 0; i<= _numRegistersMinusOne; i++){
			printRegisterName(_address + i);
			printHex(_address + i);
			Serial.print(", ");
			printHex(regData[_address + i]);
			Serial.print(", ");
			for(int j = 0; j<8; j++){
				Serial.print(bitRead(regData[_address + i], 7-j));
				if(j!=7) Serial.print(", ");
			}
			Serial.println();
		}
    }
    
}

void PicoADS1299::WREG(byte _address, byte _value) {	//  Write ONE register at _address
    byte opcode1 = _address + 0x40; 	//  WREG expects 010rrrrr where rrrrr = _address
    PicoSPI0.beginTransaction(); 				//  open SPI
    
    PicoSPI0.transfer(opcode1);					//  Send WREG command & address
    PicoSPI0.transfer(0x00);						//	Send number of registers to read -1
    PicoSPI0.transfer(_value);					//  Write the value to the register
    PicoSPI0.endTransaction(); 			//  close SPI
	regData[_address] = _value;			//  update the mirror array
	if(verbose){						//  verbose output
		Serial.print(F("Register "));
		printHex(_address);
		Serial.println(F(" modified."));
	}
}

void PicoADS1299::WREGS(byte _address, byte _numRegistersMinusOne) {
    byte opcode1 = _address + 0x40;		//  WREG expects 010rrrrr where rrrrr = _address
    PicoSPI0.beginTransaction(); 				//  open SPI
    
    PicoSPI0.transfer(opcode1);					//  Send WREG command & address
    PicoSPI0.transfer(_numRegistersMinusOne);	//	Send number of registers to read -1	
	for (int i=_address; i <=(_address + _numRegistersMinusOne); i++){
		PicoSPI0.transfer(regData[i]);			//  Write to the registers
	}	
	digitalWrite(CS,HIGH);				//  close SPI
	if(verbose){
		Serial.print(F("Registers "));
		printHex(_address); Serial.print(F(" to "));
		printHex(_address + _numRegistersMinusOne);
		Serial.println(F(" modified"));
	}
}


void PicoADS1299::updateChannelData(){
	byte inByte;
	int nchan=8;  //assume 8 channel.  If needed, it automatically changes to 16 automatically in a later block.
	PicoSPI0.beginTransaction();				//  open SPI
	
	// READ CHANNEL DATA FROM FIRST ADS IN DAISY LINE
    
	for(int i=0; i<3; i++){			//  read 3 byte status register from ADS 1 (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
		inByte = PicoSPI0.transfer(0x00);
		stat_1 = (stat_1<<8) | inByte;				
	}
	
	for(int i = 0; i<8; i++){
		for(int j=0; j<3; j++){		//  read 24 bits of channel data from 1st ADS in 8 3 byte chunks
			inByte = PicoSPI0.transfer(0x00);
			channelData[i] = (channelData[i]<<8) | inByte;
		}
	}
	
	if (isDaisy) {
		nchan = 16;
		// READ CHANNEL DATA FROM SECOND ADS IN DAISY LINE
		for(int i=0; i<3; i++){			//  read 3 byte status register from ADS 2 (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
			inByte = PicoSPI0.transfer(0x00);
			stat_2 = (stat_1<<8) | inByte;				
		}
		
		for(int i = 8; i<16; i++){
			for(int j=0; j<3; j++){		//  read 24 bits of channel data from 2nd ADS in 8 3 byte chunks
				inByte = PicoSPI0.transfer(0x00);
				channelData[i] = (channelData[i]<<8) | inByte;
			}
		}
	}
	
	PicoSPI0.endTransaction();				//  close SPI
	
	//reformat the numbers
	for(int i=0; i<nchan; i++){			// convert 3 byte 2's compliment to 4 byte 2's compliment	
		if(bitRead(channelData[i],23) == 1){	
			channelData[i] |= 0xFF000000;
		}else{
			channelData[i] &= 0x00FFFFFF;
		}
	}
}

	
//read data
void PicoADS1299::RDATA() {				//  use in Stop Read Continuous mode when DRDY goes low
	byte inByte;
	stat_1 = 0;							//  clear the status registers
	stat_2 = 0;	
	int nchan = 8;	//assume 8 channel.  If needed, it automatically changes to 16 automatically in a later block.
	PicoSPI0.beginTransaction();				//  open SPI
	PicoSPI0.transfer(_RDATA);
	
	// READ CHANNEL DATA FROM FIRST ADS IN DAISY LINE
	for(int i=0; i<3; i++){			//  read 3 byte status register (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
		inByte = PicoSPI0.transfer(0x00);
		stat_1 = (stat_1<<8) | inByte;				
	}
	
	for(int i = 0; i<8; i++){
		for(int j=0; j<3; j++){		//  read 24 bits of channel data from 1st ADS in 8 3 byte chunks
			inByte = PicoSPI0.transfer(0x00);
			channelData[i] = (channelData[i]<<8) | inByte;
		}
	}
	
	if (isDaisy) {
		nchan = 16;
		
		// READ CHANNEL DATA FROM SECOND ADS IN DAISY LINE
		for(int i=0; i<3; i++){			//  read 3 byte status register (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
			inByte = PicoSPI0.transfer(0x00);
			stat_2 = (stat_1<<8) | inByte;				
		}
		
		for(int i = 8; i<16; i++){
			for(int j=0; j<3; j++){		//  read 24 bits of channel data from 2nd ADS in 8 3 byte chunks
				inByte = PicoSPI0.transfer(0x00);
				channelData[i] = (channelData[i]<<8) | inByte;
			}
		}
	}
	
	for(int i=0; i<nchan; i++){			// convert 3 byte 2's compliment to 4 byte 2's compliment	
		if(bitRead(channelData[i],23) == 1){	
			channelData[i] |= 0xFF000000;
		}else{
			channelData[i] &= 0x00FFFFFF;
		}
	}
	
    
}



// String-Byte converters for RREG and WREG
void PicoADS1299::printRegisterName(byte _address) {
    if(_address == ID){
        Serial.print(F("ID, ")); //the "F" macro loads the string directly from Flash memory, thereby saving RAM
    }
    else if(_address == CONFIG1){
        Serial.print(F("CONFIG1, "));
    }
    else if(_address == CONFIG2){
        Serial.print(F("CONFIG2, "));
    }
    else if(_address == CONFIG3){
        Serial.print(F("CONFIG3, "));
    }
    else if(_address == LOFF){
        Serial.print(F("LOFF, "));
    }
    else if(_address == CH1SET){
        Serial.print(F("CH1SET, "));
    }
    else if(_address == CH2SET){
        Serial.print(F("CH2SET, "));
    }
    else if(_address == CH3SET){
        Serial.print(F("CH3SET, "));
    }
    else if(_address == CH4SET){
        Serial.print(F("CH4SET, "));
    }
    else if(_address == CH5SET){
        Serial.print(F("CH5SET, "));
    }
    else if(_address == CH6SET){
        Serial.print(F("CH6SET, "));
    }
    else if(_address == CH7SET){
        Serial.print(F("CH7SET, "));
    }
    else if(_address == CH8SET){
        Serial.print(F("CH8SET, "));
    }
    else if(_address == BIAS_SENSP){
        Serial.print(F("BIAS_SENSP, "));
    }
    else if(_address == BIAS_SENSN){
        Serial.print(F("BIAS_SENSN, "));
    }
    else if(_address == LOFF_SENSP){
        Serial.print(F("LOFF_SENSP, "));
    }
    else if(_address == LOFF_SENSN){
        Serial.print(F("LOFF_SENSN, "));
    }
    else if(_address == LOFF_FLIP){
        Serial.print(F("LOFF_FLIP, "));
    }
    else if(_address == LOFF_STATP){
        Serial.print(F("LOFF_STATP, "));
    }
    else if(_address == LOFF_STATN){
        Serial.print(F("LOFF_STATN, "));
    }
    else if(_address == GPIO){
        Serial.print(F("GPIO, "));
    }
    else if(_address == MISC1){
        Serial.print(F("MISC1, "));
    }
    else if(_address == MISC2){
        Serial.print(F("MISC2, "));
    }
    else if(_address == CONFIG4){
        Serial.print(F("CONFIG4, "));
    }
}

//SPI communication methods
// byte PicoADS1299::PicoSPI0.transfer(byte _data) {
// 	cli();
//     SPDR = _data;
//     while (!(SPSR & _BV(SPIF)))
//         ;
// 	sei();
//     return SPDR;
// }

// Used for printing HEX in verbose feedback mode
void PicoADS1299::printHex(byte _data){
	Serial.print("0x");
    if(_data < 0x10) Serial.print("0");
    Serial.print(_data, HEX);
}

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
