#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#define ADAFRUIT_CC3000_IRQ  3
#define ADAFRUIT_CC3000_VBAT 9
#define ADAFRUIT_CC3000_CS   10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS,
ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);

#define PUSHBUTTON 2
#define GREEN_LED A0
#define FAST      50

const int switchPin = 2;
const int ledPin = 13;
volatile int state = 0;

/**************************************************************************/
/*!
    @brief  ISR for that will set a flag to run the smart connect app
*/
/**************************************************************************/
void pushButton()
{
  state = !state;
  digitalWrite(PUSHBUTTON, state);
}

/**************************************************************************/
/*!
    @brief  Function to flash the Status LED
*/
/**************************************************************************/
void flash(int ledPin, int blinkFreq)
{
  digitalWrite(ledPin, HIGH);
  delay(blinkFreq);
  digitalWrite(ledPin, LOW);
  delay(blinkFreq);
}

/**************************************************************************/
/*!
    @brief  Run the TI smart config app
*/
/**************************************************************************/
void runSmartConfig()
{
  Serial.println(F("\nDeleting old connection profiles"));
  
  if(!cc3000.deleteProfiles())
  {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  Serial.println(F("Waiting for a SmartConfig connection (60s)"));
  
  if(!cc3000.startSmartConfig(false))
  {
    Serial.println(F("SmartConfig failed"));
    while(1)
      flash(GREEN_LED, FAST);
  }
    
  Serial.println(F("Saved connection details and connected to AP"));
    
  //Wait for DHCP to complete
  Serial.println(F("Request DHCP"));
    
  while(!cc3000.checkDHCP())
  {
    delay(100); //To Do: Insert a DHCP timeout
  }
    
  while(!displayConnectionDetails())
  {
    delay(1000);
  }
  
  Serial.println(F("\nTo use these connection details be sure to use"));
  Serial.println(F("'.begin(false, true)' with your Adafruit_CC3000"));
  Serial.println(F("code instead of the default '.begin()' values!"));  
  Serial.println(F("\nClosing the connection"));
  cc3000.disconnect();
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

/**************************************************************************/
/*!
    @brief  Displays the driver mode (tiny of normal), and the buffer
            size if tiny mode is not being used

    @note   The buffer size and driver mode are defined in cc3000_common.h
*/
/**************************************************************************/
void displayDriverMode(void)
{
  #ifdef CC3000_TINY_DRIVER
    Serial.println(F("CC3000 is configure in 'Tiny' mode"));
  #else
    Serial.print(F("RX Buffer : "));
    Serial.print(CC3000_RX_BUFFER_SIZE);
    Serial.println(F(" bytes"));
    Serial.print(F("TX Buffer : "));
    Serial.print(CC3000_TX_BUFFER_SIZE);
    Serial.println(F(" bytes"));
  #endif
}

/**************************************************************************/
/*!
    @brief  Tries to read the CC3000's internal firmware patch ID
*/
/**************************************************************************/
uint16_t checkFirmwareVersion(void)
{
  uint8_t major, minor;
  uint16_t version;
  
#ifndef CC3000_TINY_DRIVER  
  if(!cc3000.getFirmwareVersion(&major, &minor))
  {
    Serial.println(F("Unable to retrieve the firmware version!\r\n"));
    version = 0;
  }
  else
  {
    Serial.print(F("Firmware V. : "));
    Serial.print(major); Serial.print(F(".")); Serial.println(minor);
    version = major; version <<= 8; version |= minor;
  }
#endif
  return version;
}

/**************************************************************************/
/*!
    @brief  Tries to read the 6-byte MAC address of the CC3000 module
*/
/**************************************************************************/
void displayMACAddress(void)
{
  uint8_t macAddress[6];
  
  if(!cc3000.getMacAddress(macAddress))
  {
    Serial.println(F("Unable to retrieve MAC Address!\r\n"));
  }
  else
  {
    Serial.print(F("MAC Address : "));
    cc3000.printHex((byte*)&macAddress, 6);
  }
}

/**************************************************************************/
/*!
    @brief  The Main Routine
*/
/**************************************************************************/
void setup()
{
  attachInterrupt(0, pushButton, RISING);
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  
  Serial.println(F("\nInitializing the CC3000 ..."));
  displayDriverMode();
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);  
  
  Serial.println(F("Trying to reconnect using SmartConfig values ..."));
  
  if(!cc3000.begin(false, true))
  {
    Serial.println(F("Unable to initialize..."));
    Serial.println(F("You need an ssid & password. Starting Smart Config App."));
    runSmartConfig(); //Run the smart config app
    Serial.println(F("Done setting up ssid & password. Please restart module."));
    while(1);
  }

  uint16_t firmware = checkFirmwareVersion();
  if ((firmware != 0x113) && (firmware != 0x118)) 
  {
    Serial.println(F("Wrong firmware version!"));
    while(1);
  }  
  
  Serial.println(F("Reconnected!"));
  
  Serial.println(F("\nRequesting DHCP"));
  while(!cc3000.checkDHCP())
  {
    delay(100); //To Do: Insert a DHCP timeout
  }
  
  while(!displayConnectionDetails())
  {
    delay(1000);
  }
  
  Serial.println(F("\nClosing the connection"));
    cc3000.disconnect();
}

/**************************************************************************/
/*!
    @brief  The Main Loop
*/
/**************************************************************************/
void loop()
{
  //1. Check if the button is pressed to run smart config!
  if(state == HIGH)
    runSmartConfig();
    while(1);
    
  //Your code goes here  
}
