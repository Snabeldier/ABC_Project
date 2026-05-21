Instuctions to setup the IDE:

Make an account at ARM/KEIL and follow the instructions here: https://www.keil.arm.com/mdk-community/

Download the Ambiq Suite SDK and unpack it somewhere on your computer (https://contentportal.ambiq.com/)
Use the package installer from uVision and download the Ambiq device support package (maybe the use of Version 1.3.0 is required: 1.4.1 may cause some problems)
Open the ApolloFirmware and rightclick on the lefthand side target 1 and select options for target. Click on C/C++ and add the defines : AM_PACKAGE_BGA AM_PART_APOLLO3
In the same window: open the include paths and adjust the location for the library and Ambiq Suite
Click on Debug: select J-Link for he used probe, click on settings and choose PORT: SW
The files in the folder lib & utils may need to be re-added: delete all of the them and add them again with your Ambiq Suite location (AmbiqSuite_R3.0.0\mcu\apollo3\hal) & (AmbiqSuite_R3.0.0\utils)
Once this is done compilation should be possible. If not, check the following things:
a.	Browse to the am_hal_debug.h file and search for DBG_FILENAME and replace MODULE with FILE_NAME in line 92. For further reference look https://developer.arm.com/documentation/100068/0618/Migrating-from-armcc-to-armclang/Migrating-predefined-macros

If the core_cm4.h indicates errors, the is probably a problem with the Ambiq device support package: quick fix could be copying the RTE folder of project from the Gitea