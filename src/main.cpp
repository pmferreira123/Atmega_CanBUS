#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>
#include <mcp_can.h>


//struct can_frame canMSG;
const long unsigned int Semtke_Test = 0x8C04BBDA; // CAN ID for CC Module Test Total Voltage
const long unsigned int Semtke_01 = 0x8C04BBC0; // CAN ID for CC Module Test Total Voltage
const long unsigned int Semtke_03  = 0x8C04BBC2; // CAN ID for CC Module Test Total Voltage
const long unsigned int SOC_Semtke  = 0x8774BB00; // CAN ID for SOC CC Module Test Total Voltage
const unsigned short int num_string_group = 1; // Number of string groups
const float Max_Value_CC = 135; // Maximum voltage of the CC module
const float Min_Value_CC = 60; // Minimum voltage of the CC module
const unsigned long Pow_Min_Value_CC = pow(Min_Value_CC, 2); // Pre-calculate the square of the minimum voltage for efficiency
const unsigned long Denominator_CC = pow(Max_Value_CC, 2) - pow(Min_Value_CC, 2); // Denominator for the state of charge calculation
float CC_Voltage_Factor = 0.008; // 8mV per bit, for a 12-bit ADC with a reference voltage of 3.3V, the factor is 3.3/4096 = 0.0008056640625 V per bit, which is approximately 0.008 V per bit when rounded to three decimal places.
short unsigned total_voltage = 0;
float fl_voltage = 0.0;


// CAN TX Variables
unsigned long prevTX = 0;                                        // Variable to store last execution time
const unsigned int invlTX = 1000;                                // One second interval constant
//byte data[] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x55};  // Generic CAN data to send
byte SoC[] = {0, 0}; // empty data array to send, since we only care about the state of charge variable
byte* pSoC= &SoC[0]; // pointer to the state of charge



// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

// Serial Output String Buffer
char msgString[128];

// CAN0 INT and CS
#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);   // Set CS to pin 10

/* ---------------------------------------------------------------------------------------*/
signed char CC_MSoC(unsigned short int x) {  // devolve em % o SoC de um modulo de CC

  float SOC = (( pow(x, 2) - Pow_Min_Value_CC ) / Denominator_CC) * 100; // Calculate the state of charge using the quadratic formula
  int SOC_int = (int)(SOC);
  float decpart = SOC - SOC_int; // Get the decimal part of the state of charge
  if (decpart >= 0.5) { // If the decimal part is 0.5 or greater, round up the state of charge
    SOC_int += 1;
  }
  return SOC_int;
}

/*----------------------------------------------------------------------------------------*/


void setup() {
   Serial.begin(115200);  // CAN is running at 500,000BPS; 115,200BPS is SLOW, not FAST, thus 9600 is crippling.
    
  // Since we do not set NORMAL mode, we are in loopback mode by default.
  //CAN0.setMode(MCP_NORMAL);

  
pinMode(CAN0_INT, INPUT);    
                       // Configuring pin for /INT input
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    CAN0.setMode(MCP_NORMAL);
      Serial.println("MCP2515 Initialized Successfully!");
      
    }
  else {
    Serial.println("Error on initializing MCP2515...");
  }

}


void loop() {

if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if( (rxId  ==  Semtke_01) || (rxId  ==  Semtke_03) || (rxId  ==  Semtke_Test) )   {  // Determine if ID is CC Module Total Voltage

      //sprintf(msgString, "Extended ID8: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
      sprintf(msgString, "Message Received : 0x%.8lX", rxId );
      Serial.println(msgString);
       fl_voltage = 0.0;    
       fl_voltage = ((rxBuf[1] << 8) + rxBuf[0]) * CC_Voltage_Factor; // Combine the first two bytes to get the voltage value

       total_voltage = (int)fl_voltage; // Convert float to integer representation (e.g., 12.34V becomes 12)
       float decpart = fl_voltage - total_voltage; // Get the decimal part of the voltage
       if (decpart >= 0.5) { // If the decimal part is 0.5 or greater, round up the total voltage
         total_voltage += 1;
       }
       sprintf(msgString, "Message Voltage: %u V ", total_voltage);
       Serial.println(msgString);
       //aux = CC_MSoC(total_voltage);
       //SoC[0] = (byte)(aux >> 8); // Calculate the state of charge and store it in the variable pointed to by pSoC
       //SoC[1] = (byte)(aux & 0xFF); 
       SoC[0] = (byte)CC_MSoC(total_voltage); // Calculate the state of charge and store it in the variable pointed to by pSoC
      
    }
    // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
    //SoC = (byte)CC_MSoC(135); // calculate the state of charge for a given voltage.
    byte sndStat = CAN0.sendMsgBuf(SOC_Semtke, 1, num_string_group, pSoC);
    if(sndStat == CAN_OK){
      //*pSoC=135; update the state of charge variable to a new value, to be sent in the next CAN message
      sprintf(msgString, "Message: %u%% ", *pSoC);
      Serial.print(msgString);
      Serial.println("Message Sent Successfully!");
    } 
    else {
      Serial.println("Error Sending Message...");
    }
   }
    
  else Serial.println("There are no messages on Can Bus...");
  delay(500);   // send data per 500ms
/*

   if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
      
  }*/


}
