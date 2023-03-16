
#include <PicoADS1299.h>



/*Original code developed by Joel Murphy and Conor Russomanno (Summer 2013) for Arduino Uno.
 * Modded by Indrayudd Roy Chowdhury (Spring 2023)
  This example uses the PicoADS1299 Arduino Library, a software bridge between the ADS1299 TI chip and 
  Raspberry Pi Pico.


  
  
  Pi Pico - Pin Assignments
  SCK = 2
  MISO [DOUT] = 4
  MOSI [DIN] = 3
  CS = 5; 
  RESET = 9;
  DRDY = 8;
  
*/

//#include "PicoADS1299.h"
#include <filters.h>
//#include <FIRFilter.h> 

PicoADS1299 ADS;                           // create an instance of ADS1299

unsigned long thisTime;                
unsigned long thatTime;
unsigned long elapsedTime;
int resetPin = 9;                      // pin 9 used to start conversion in Read Data Continuous mode
int sampleCounter = 0;                 // used to time the tesing loop
boolean testing = true;               // this flag is set in serialEvent on reciept of prompt



void setup() {
//  delay(5000);
  // don't put anything before the initialization routine for recommended POR  
  ADS.initialize(8,9,5,3, false); // (DRDY pin, RST pin, CS pin, SCK frequency in MHz);

  Serial.begin(115200);
  //Serial.println("ADS1299-Arduino UNO Example 2"); 
  delay(2000);             

  ADS.verbose = true;      // when verbose is true, there will be Serial feedback 
  ADS.RESET();             // send RESET command to default all registers
  ADS.SDATAC();            // exit Read Data Continuous mode to communicate with ADS
  //ADS.WREG(ID,0b00000001);
  //ADS.RREG(ID);
  ADS.WREG(CONFIG1,0b11010110);
  ADS.WREG(CONFIG2,0b11010101);
  ADS.WREG(CONFIG3,0b11001010);
  ADS.WREG(CH1SET,0b10100000);
  ADS.WREG(CH2SET,0b00100000);
  ADS.WREG(CH3SET,0b00100000);
  ADS.WREG(CH4SET,0b00100000);
  ADS.WREG(CH5SET,0b00100000);
  ADS.WREG(CH6SET,0b00100000);
  ADS.WREG(CH7SET,0b00100000);
  ADS.WREG(CH8SET,0b00100000);
  ADS.WREG(BIAS_SENSP,0b11111111);
  ADS.WREG(BIAS_SENSN,0b11111111);
  ADS.WREG(LOFF_SENSP,0x00);
  ADS.WREG(LOFF_SENSN,0x00);
  //ADS.WREG(MISC1,0b00100000);
  ADS.WREG(CONFIG4,0x02);
  
  ADS.RREGS(0x00,0x17);     // read all registers starting at ID and ending at CONFIG4
//  ADS.WREG(CONFIG3,0xE0);  // enable internal reference buffer, for fun
//  ADS.RREG(CONFIG3);       // verify write
  ADS.RDATAC();            // enter Read Data Continuous mode
  ADS.START();                    // start sampling at the default rate
  
  //Serial.println("Press 'x' to begin test");    // ask for prompt
} // end of setup
unsigned long old_time;
unsigned long new_time;
void loop(){

//  delay(26);
   new_time = millis();
  
  if (testing){
    //Serial.println("entering test loop");
    while(digitalRead(8)){            // watch the DRDY pin
      }
    ADS.updateChannelData();          // update the channelData array
//    int ads_value_1 =("Channel 1",ADS.channelData[0]);
    int ads_value_2 =("Channel 2",ADS.channelData[1]);
    int ads_value_3 =("Channel 3",ADS.channelData[2]);
    int ads_value_4 =("Channel 4",ADS.channelData[3]);
    int ads_value_5 =("Channel 5",ADS.channelData[4]);
    int ads_value_6 =("Channel 6",ADS.channelData[5]);
    int ads_value_7 =("Channel 7",ADS.channelData[6]);
    int ads_value_8 =("Channel 8",ADS.channelData[7]);
    

      
      Serial.print(ads_value_8);
      Serial.print("\t");
      Serial.print(ads_value_3);
      Serial.print("\t");
      Serial.print(ads_value_4);
      Serial.print("\t");
      Serial.print(ads_value_5);
      Serial.print("\t");
      Serial.print(ads_value_6);
      Serial.print("\t");
      Serial.println(ads_value_7);

  }// end of testing

//  ADS.STOP();                     // stop the sampling
  
} // end of loop


void serialEvent(){            // send an 'x' on the serial line to trigger ADStest()
  while(Serial.available()){      
    char inChar = (char)Serial.read();
    if (inChar  == 'x'){
      testing = false;
    }
    if(inChar == 'v'){
      testing = true;
    }
  }
}
