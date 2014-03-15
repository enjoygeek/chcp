/* 
 * Central Heating Control Panel
 * Copyright 2014 Nicholas Humfrey
 *
 * Libary Dependencies:
 *   https://github.com/jcw/ethercard
 *   https://github.com/snigelen/pcd8544
 * 
 */

#include <Wire.h>
#include <EtherCard.h>
#include <pcd8544.h>

#define LCD_RST_PIN  5
#define RED_LED_PIN  6
#define BUTTON_PIN   7
#define ETH_SS_PIN   8
#define LCD_DC_PIN   9
#define LCD_SS_PIN   10

#define DEBOUNCE_TIME 300



// Create a pcd8544 object using hardware SPI
pcd8544 lcd(
  LCD_DC_PIN,
  LCD_RST_PIN,
  LCD_SS_PIN
);

// Ethernet and MAC address buffers
byte Ethernet::buffer[700];
static byte mymac[6];


void setup(void)
{
  // Setup LED and buttons
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  // Clear the LCD
  lcd.begin();
  lcd.clear();

  // Start I2C communication and read MAC address
  Wire.begin();
  readMacAddress(0x0, mymac);
  delay(500);
  
  // Setup IP networking
  startNetworking();
  delay(2000);

  lcd.clear();
  lcd.println("Ready");
}



int state = LOW;      // the current state of the output pin
int previous = LOW;   // the previous reading from the input pin
long time = 0;        // the last time the output pin was toggled

void loop(void)
{
  word rc;

  // Process any incoming ethernet packets
  rc = ether.packetLoop(ether.packetReceive());

  // Check if button has gone from LOW to HIGH and stayed there
  int reading = digitalRead(BUTTON_PIN);
  if (reading == HIGH && previous == LOW && millis() - time > DEBOUNCE_TIME) {
    state = !state;
    time = millis();    
  }

  digitalWrite(RED_LED_PIN, state);

  previous = reading;
}


// Setup Ethetnet networking
void startNetworking()
{
  lcd.clear();
  lcd.print("Setup Ethernet");
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8)) {
    lcd.println("OK");
  } else {
    lcd.println("FAILED");
    while (true);
  }

  lcd.println("Setup DHCP");
  if (ether.dhcpSetup()) {
    lcd.println("OK");
  } else {
    lcd.println("FAILED");
    while (true);
  }

  // Display our IP address
  lcd.println("IP Address:");
  for (byte i = 0; i < 4; ++i) {
    lcd.print( ether.myip[i], DEC );
    if (i < 3)
      lcd.print('.');
  }
  lcd.println();
}


// Function to read the MAC address from a AT24MAC402 EEPROM
//
// address: the 3-bit address assinged to the EEPROM (using the A0 A1 A2 pins)
// macaddr: a pointer to 6 bytes of memory
//
void readMacAddress(byte address, byte macaddr[]) {
  int i2c_address = 0x58 | address;
 
  // Tell the EEPROM where we would like to read from
  Wire.beginTransmission(i2c_address);
  Wire.write(0x9A); // Location of the EUI-48
  Wire.endTransmission();
 
  // Now read 6 bytes from that memory address
  Wire.requestFrom(i2c_address, 6);
  for(byte i=0; i<6; i++) {
    macaddr[i] = Wire.read();
  }
}

