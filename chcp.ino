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

uint8_t bitmap_on[3*60] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0xe0,0xf0,0xf0,0xf8,0xf8,
  0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf0,0xe0,0xe0,0xc0,0x00,0x00,0x00,
  0x00,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf0,0xe0,0xc0,0x80,0x00,0x00,0x00,0x00,
  0x00,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0xff,0xff,0xff,0xff,0xc7,0x01,
  0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xc7,0xff,0xff,0xff,0xff,0xff,0x7c,0x00,
  0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x0f,0x1f,0x3f,0x7f,0xff,0xfe,0xfc,0xf8,
  0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x07,0x0f,0x1f,0x1f,0x3f,0x3f,
  0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3f,0x3f,0x1f,0x1f,0x0f,0x07,0x03,0x00,0x00,
  0x00,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0f,
  0x1f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

uint8_t bitmap_off[3*60] PROGMEM = {
  0x00,0x00,0x00,0x80,0xc0,0xe0,0xf0,0xf0,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,
  0xf8,0xf8,0xf8,0xf0,0xe0,0xe0,0xc0,0x00,0x00,0x00,0x00,0x00,0xf8,0xf8,0xf8,
  0xf8,0xf8,0xf8,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x00,0x00,0x00,0xf8,
  0xf8,0xf8,0xf8,0xf8,0xf8,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x00,0x00,
  0x00,0x00,0xfe,0xff,0xff,0xff,0xff,0xff,0xc7,0x01,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01,0xc7,0xff,0xff,0xff,0xff,0xff,0x7c,0x00,0x00,0x00,0xff,0xff,0xff,
  0xff,0xff,0xff,0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x00,0x00,0x00,0xff,
  0xff,0xff,0xff,0xff,0xff,0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x00,0x00,
  0x00,0x00,0x00,0x03,0x07,0x0f,0x1f,0x1f,0x3f,0x3f,0x3e,0x3e,0x3e,0x3e,0x3e,
  0x3e,0x3f,0x3f,0x1f,0x1f,0x0f,0x07,0x03,0x00,0x00,0x00,0x00,0x3f,0x3f,0x3f,
  0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
  0x3f,0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


#define BUTTON_PIN   5
#define RED_LED_PIN  6
#define ETH_SS_PIN   7
#define LCD_RST_PIN  8
#define LCD_SS_PIN   9
#define LCD_DC_PIN   10

#define DEBOUNCE_TIME 300

enum {
  STATE_UNKNOWN,
  STATE_OFF,
  STATE_ON,
  STATE_ERROR,
};

int state = STATE_UNKNOWN;    // the current state of the output pin
int buttonPrevious = LOW;     // the previous reading from the input pin
long buttonTime = 0;          // the last time the output pin was toggled

uint8_t serverIp[4] = {0,0,0,0};


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
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Clear the LCD
  lcd.begin();
  lcd.clear();
  lcd.gotoRc(0,0);

  // Start I2C communication and read MAC address
  Wire.begin();
  readMacAddress(0x0, mymac);

  lcd.println("Central");
  lcd.println("Heating");
  lcd.println("Control");
  lcd.println("Panel");
  lcd.println("MAC Address:");
  for (byte i = 0; i < 6; ++i) {
    lcd.print(mymac[i], HEX);
  }
  lcd.println();
  delay(2000);

  // Setup IP networking
  startNetworking();
  delay(2000);

  // Update the LCD screen to current state
  updateDisplay();
}


void loop(void)
{
  word rc;

  // Process any incoming ethernet packets
  rc = ether.packetLoop(ether.packetReceive());

  // Check if button has gone from LOW to HIGH and stayed there
  int reading = digitalRead(BUTTON_PIN);
  if (reading == LOW && buttonPrevious == HIGH && millis() - buttonTime > DEBOUNCE_TIME) {
    const char* message = NULL;
    buttonTime = millis();

    if (state == STATE_ON) {
      message = "off";
    } else {
      message = "on";
    }

    ether.sendUdp(message, strlen(message), 1886, serverIp, 1886);
  }

  buttonPrevious = reading;
}


void updateDisplay()
{
  lcd.clear();
  lcd.gotoRc(0,0);
  lcd.println("    Central  ");
  lcd.println("    Heating  ");
  lcd.println();

  if (state == STATE_ON) {
    lcd.gotoRc(3, 12);
  	lcd.bitmap(bitmap_on, 3, 60);
    digitalWrite(RED_LED_PIN, HIGH);
  } else if (state == STATE_OFF) {
    lcd.gotoRc(3, 12);
  	lcd.bitmap(bitmap_off, 3, 60);
    digitalWrite(RED_LED_PIN, LOW);
  } else if (state == STATE_ERROR) {
    lcd.println("Error");
    digitalWrite(RED_LED_PIN, LOW);
  } else if (state == STATE_UNKNOWN) {
    lcd.println("Please wait.");
    digitalWrite(RED_LED_PIN, LOW);
  }
}


// Checks the UDP packet and updates state global variable
void udpUpdateState(word port, byte ip[4], const char *data, word len) {
  if (len == 2 && data[0] == 'o' && data[1] == 'n') {
    state = STATE_ON;
  } else if (len == 3 && data[0] == 'o' && data[1] == 'f' && data[2] == 'f') {
    state = STATE_OFF;
  } else {
    state = STATE_ERROR;
  }

  updateDisplay();
}


void dhcpNtpOptionCallback(uint8_t option, const byte* ptr, uint8_t len)
{
    EtherCard::copyIp(serverIp, (uint8_t*)ptr);
}


// Setup Ethetnet networking
void startNetworking()
{
  lcd.clear();
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) < 1) {
    lcd.println("Ethernet Fail");
    while (true);
  }

  // Register callback for DHCP option 183
  ether.dhcpAddOptionCallback(183, dhcpNtpOptionCallback);

  lcd.println("Setup DHCP");
  if (ether.dhcpSetup()) {
    lcd.println("OK");
  } else {
    lcd.println("FAILED");
    while (true);
  }

  // Display our IP address
  lcd.println("My Address:");
  for (byte i = 0; i < 4; ++i) {
    lcd.print( ether.myip[i], DEC );
    if (i < 3)
      lcd.print('.');
  }
  lcd.println();

  // Display the server address
  lcd.println("Server Addr:");
  for (byte i = 0; i < 4; ++i) {
    lcd.print( serverIp[i], DEC );
    if (i < 3)
      lcd.print('.');
  }
  lcd.println();

  // Register udpUpdateState() to port 1886
  ether.udpServerListenOnPort(&udpUpdateState, 1886);
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

