This is the modified LoRaWAN LoRaMac-node from https://github.com/Lora-net/LoRaMac-node

A implementation for the Ambiq Apollo 3 is added to the src folder. The implementation requires the following peripherals to be present:
* MB85RC64TAPNF-G-BDERE1 FRAM 
* AM1805 external RTC

It is specifically made for the use with a LAMBDA62-8S board and is used by the Apollo3_LoRaMac repository

File Overview:
* board.c : board/cpu control for LoRaWAN: init functions important, rest is partially not implemented
* eeprom-board.c : Important for NVM storage management. If different FRAM/NVM is used, this must be changed accordingly
* gpio-board.c : not implemented, own gpio library used instead
* pinName-board.c : not implemented, own gpio/pins used
* pinName-ioe.c : not implemented, own gpio/pins used
* rtc-board.c : very important for all LoRaWAN timings: is currently build using the internal RTC of the apollo, not really desirable to use the external one
* spi-board.c : not implemented, own spi lib used
* sx126x-board.c : very important, should work correctly sofar, handles all the communication of the stack with the module using SPI
* transmitConfig.h : partially used, ideally the whole stack can be configured using this file, however this is not implemented fully
* utilities.c : generic implementation from Semtech