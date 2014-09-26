chcp
====

PoE powered Central Heating Control Panel

![Block Diagram](https://raw.github.com/njh/chcp/master/block-diagram.png "Block Diagram")



ATMEGA328 Pin Mapping
----------------------

| Pin | Port | Arduino | Function    |
|-----|------|---------|-------------|
| 1   | RST# |         |             |
| 2   | D0   | D0      | Serial RX   |
| 3   | D1   | D1      | Serial TX   |
| 4   | D2   | D2      |             |
| 5   | D3   | D3      | LCD LIGHT   |
| 6   | D4   | D4      |             |
| 7   | VCC  |         |             |
| 8   | GND  |         |             |
| 9   | CLK  |         |             |
| 10  | CLK  |         |             |
| 11  | D5   | D5      |             |
| 12  | D6   | D6      |             |
| 13  | D7   | D7      | Ethernet SS |
| 14  | B0   | D8      | LCD RESET   |
| 15  | B1   | D9      | LCD SS      |
| 16  | B2   | D10     | LCD DC      |
| 17  | B3   | D11     | SPI MOSI    |
| 18  | B4   | D12     | SPI MISO    |
| 19  | B5   | D13     | SPI SCLK    |
| 20  | VCC  |         |             |
| 21  | Aref |         |             |
| 22  | GND  |         |             |
| 23  | C0   | A0      |             |
| 24  | C1   | A1      |             |
| 25  | C2   | A2      |             |
| 26  | C3   | A3      |             |
| 27  | C4   | A4      | I2C SDA     |
| 28  | C5   | A5      | I2C SCL     |


PoE Pin Mapping
---------------

| RJ45 | IN28J60 | PEM1300 |
|------|---------|---------|
| 1+2  | POE+1   | VA1     |
| 3+6  | POE-1   | VA2     |
| 4+5  | POE-2   | VB1     |
| 7+8  | POE+2   | VB2     |
