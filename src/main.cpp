#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>
#include <mcp_can.h>


//struct can_frame canMSG;
//struct MCP2515 mcp2515(5);

// CAN TX Variables
unsigned long prevTX = 0;                                        // Variable to store last execution time
const unsigned int invlTX = 1000;                                // One second interval constant
byte data[] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x56};  // Generic CAN data to send

// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

// Serial Output String Buffer
char msgString[128];

// CAN0 INT and CS
#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

void setup() {
   Serial.begin(115200);  // CAN is running at 500,000BPS; 115,200BPS is SLOW, not FAST, thus 9600 is crippling.
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  // Since we do not set NORMAL mode, we are in loopback mode by default.
  //CAN0.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT);                           // Configuring pin for /INT input
  
  Serial.println("MCP2515 Library Loopback Example...");
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

