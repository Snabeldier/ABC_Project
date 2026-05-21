This repository contains the LoRaWAN implementation for the Ambiq Apollo 3
It can be used on both the development board and the mini board.
It is based on the periodic uplink example from the Semtech/Stackforce library for the SX1261/62 and is adjusted to do the following:

* ABP procedure for the TTN application
* The security credentials can be changed in the se-identity.h - if this is done for a sensornode already in use, the NvmDataMgmtFactoryReset() needs to be called once to erase transmission statistics
* Currently the implemented procedure is the following: if there is no real time saved on the external RTC, the software first requests the current time from the TTN. Afterwards it continously measures and transmits data
* The data is send using a data format called JalapenosLpp which is based on the discontinued CayenneLpp. A decoder is available for TTN and can be found in this repository which can be added to the TTN device
* Currently the PAR(AS7341), temperature(SHTC-3), humidity(SHTC-3), shuntVoltage(INA232), busVoltage(INA232), currentRegister(INA232), SuperCapVoltage(ADC) and timestamp(AM1805) are measured and transmitted



Requirements for operation:

* Ambiq AM 1805 external RTC for timekeeping and reduced current consumption: The stack in general works without the RTC but looses the ability the save power while maintaining the current time
* the FRAM is required as non volatile memory for the LoRaWAN stack: Without the external FRAM the Apollo 3s internal FLASH memory needs to be used: implementation of this is required
* LAMBDA62-8S board: some specifics in the implementation need to be reverted to work with a different LoRaWAN module: e.g. the LDO is used instead of the (non existing) DCDC converter, the Antenna switch logic



Further Implementation work:

* A implementation that sends a reduced dataset when the whole dataset is too long for various circumstances is desireable to increase device robustness
* A (bigger) FRAM implementation that stores measurement results on the FRAM for increased robustness incase of transmission failure
* The external RTC as powersaving device for the sensor node is not implemented yet

Notes on how to use this project:

* The Apollo\_LoRaMac\_Library and the Apollo3\_Library are both required to be present
* The project is used with Keil uVision. Some instructions on how to set-up the IDE and the project are provided in the Apollo3\_Firmware repository
* Addiotionally to the instructions there, the filelocations for the LoRaMac library need to be taken care off
* SOFT\_SE REGION\_EU868 ACTIVE\_REGION=LORAMAC\_REGION\_EU868 need to be added to the preprocessor symbols
* Maybe the files need to be added again

