Library that can be used to implement sensors and other applications on the Ambiq Apollo 3. Already supports the AS7341 and SHTC3 sensors, DAC63002 Digital-to-Analog-Converter, MB85RC64TA FRAM memory and the AM1805 external RTC.
Goal of this library is that a solid foundation of functionalities like timekeeping memory management is given to other people working with the EcoSense Microcontroller Platform based on the Ambiq Apollo 3

Detailed list of provided functionalities:
* Provide drivers for SPI and I2C communication
* Provide drivers for RTC: timekeeping and sleeping (set in seconds)
* Provide drivers for the DAC63002
* Provide drivers for external FRAM
* Provide driver & measurement protocol for PAR using the AS7341 sensor build on included I2C Library
* Provide driver & measurement protocol for temperature/humidity using the SHTC3 sensor build on included I2C Library
* Provide a pin assignment file with all GPIO just named by their connected board
* Added source/header for a task handler where sensor implementations are supposed to be added. The implementation for the AS7341 can be used as an example on how the use of the task handler is intended
